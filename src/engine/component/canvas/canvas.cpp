#include "canvas.h"

#include "src/engine/scene/scene.h"
#include "src/util/serialization_util.h"

using namespace prt3;

Canvas::Canvas(Scene & /*scene*/, NodeID node_id)
 : m_node_id{node_id} {}

Canvas::Canvas(Scene & /*scene*/, NodeID node_id, std::istream & in)
 : m_node_id{node_id} {
    read_stream(in, m_layer);
}

void Canvas::serialize(
    std::ostream & out,
    Scene const & /*scene*/
) const {
    write_stream(out, m_layer);
}


glm::vec2 get_anchor_factor(CanvasNode::AnchorPoint anchor_point) {
    switch (anchor_point) {
        case CanvasNode::AnchorPoint::top_left: {
            return glm::vec2{0.0f, 0.0f};
        }
        case CanvasNode::AnchorPoint::top: {
            return glm::vec2{0.5f, 0.0f};
        }
        case CanvasNode::AnchorPoint::top_right: {
            return glm::vec2{1.0f, 0.0f};
        }
        case CanvasNode::AnchorPoint::mid_left: {
            return glm::vec2{0.0f, 0.5f};
        }
        case CanvasNode::AnchorPoint::mid: {
            return glm::vec2{0.5f, 0.5f};
        }
        case CanvasNode::AnchorPoint::mid_right: {
            return glm::vec2{1.0f, 0.5f};
        }
        case CanvasNode::AnchorPoint::bottom_left: {
            return glm::vec2{0.0f, 1.0f};
        }
        case CanvasNode::AnchorPoint::bottom: {
            return glm::vec2{0.5f, 0.5f};
        }
        case CanvasNode::AnchorPoint::bottom_right: {
            return glm::vec2{1.0f, 0.5f};
        }
    }
}

static bool is_whitespace(char c) {
    // TODO: Handle all whitespace (though in practice we only expect space)
    return c == ' ' || c == '\n';
}

static float get_curr_word_width(
    CanvasText const & text,
    size_t index
) {
    /* Note: Function is technically misnamed. We care about the width of the
     *       word and any following punctuation.
     */
    float width = 0.0f;

    char const * str = text.text;

    /* We do not stop at text.length since we want formatting to be consistent
     * when incrementally displaying a string across several frames.
     */
    while (str[index] && !is_whitespace(str[index])) {
        unsigned char c =
            *reinterpret_cast<unsigned const char *>(str[index]);
        width += text.font_size * text.char_info[c].advance;
        ++index;
    }

    return width;
}

void Canvas::collect_render_data(Scene const & scene, std::vector<RenderRect2D> & data) {
    struct StackInfo {
        glm::vec4 color;
        int32_t layer;
        glm::vec2 dimension;
        glm::vec2 position;
    };

    thread_local std::vector<StackInfo> stack_info;
    stack_info.resize(1);
    stack_info[0].color = glm::vec4{1.0f, 1.0f, 1.0f, 1.0f};
    stack_info[0].layer = 0;

    unsigned int w, h;
    scene.get_window_size(w, h);
    stack_info[0].dimension = glm::vec2{w, h};

    int32_t curr_parent = -2;
    for (CanvasStackNode const & sn : m_node_stack) {
        CanvasNode const & n = sn.n;
        bool moved_up_stack = curr_parent < sn.parent;
        curr_parent = sn.parent;

        if (!moved_up_stack) {
            stack_info.pop_back();
        }

        StackInfo const & curr_info = stack_info.back();
        StackInfo info;

        /* color */
        glm::vec4 color;
        if (n.inherit_color) {
            color = curr_info.color * n.color;
        } else {
            color = n.color;
        }
        info.color = color;

        /* dimension */
        glm::vec2 dimension;
        switch (n.dimension_mode) {
            case CanvasNode::UnitType::absolute: {
                dimension = n.dimension;
                break;
            }
            case CanvasNode::UnitType::relative: {
                dimension = curr_info.dimension * n.dimension;
                break;
            }
        }
        info.dimension = dimension;

        /* position */
        glm::vec2 position = curr_info.position +
                             get_anchor_factor(n.parent_anchor) *
                             curr_info.dimension;

        switch (n.position_mode) {
            case CanvasNode::UnitType::absolute: {
                position += n.position;
                break;
            }
            case CanvasNode::UnitType::relative: {
                position += curr_info.dimension * n.position;
                break;
            }
        }

        position += -get_anchor_factor(n.center_point) * dimension;

        switch (n.origin_mode) {
            case CanvasNode::UnitType::absolute: {
                position += n.origin;
                break;
            }
            case CanvasNode::UnitType::relative: {
                position += -dimension * n.origin;
                break;
            }
        }

        info.position = position;

        /* layer */
        int32_t layer = int32_t{curr_info.layer} + int32_t{n.layer};
        info.layer = layer;
        int32_t rect_layer = layer + m_layer * (UINT16_MAX + 1);

        switch (n.mode) {
            case CanvasNode::Mode::rect: {
                RenderRect2D rect;
                rect.color = color;
                rect.uv0 = n.u.rect.uv0;
                rect.uv1 = n.u.rect.uv1;
                rect.uv2 = n.u.rect.uv2;
                rect.uv3 = n.u.rect.uv3;
                /* convert to view space coordinates */
                rect.position = 2.0f * (position / glm::vec2{w, h}) - 1.0f;
                rect.dimension = 2.0f * (dimension / glm::vec2{w, h});
                rect.texture = n.texture;
                rect.layer = rect_layer;

                data.push_back(rect);

                break;
            }
            case CanvasNode::Mode::text: {
                CanvasText const & text = n.u.text;
                float font_size = static_cast<float>(text.font_size);

                glm::vec2 curr_pos = position;
                curr_pos.y += dimension.y - font_size;

                float limit_x = position.x + dimension.x;

                size_t data_curr = data.size();
                data.resize(data_curr + text.length);

                for (uint32_t i = 0; i < text.length; ++i) {
                    unsigned char c =
                        *reinterpret_cast<unsigned const char *>(&text.text[i]);

                    FontChar const & fc = text.char_info[c];
                    glm::vec2 o = fc.uv_origin;
                    glm::vec2 dim = fc.uv_dimension;

                    float lb = font_size * fc.left_bearing;
                    float advance = font_size * fc.advance;

                    glm::vec2 rect_dim =
                        fc.norm_scale *
                        glm::vec2{font_size * fc.ratio, font_size};

                    bool first_char_in_word =
                        !is_whitespace(c) &&
                        (i == 0 || is_whitespace(text.text[i-1]));

                    if (first_char_in_word) {
                        float curr_word_width = get_curr_word_width(text, i);
                        if (curr_pos.x + curr_word_width > limit_x) {
                            curr_pos.x = position.x;
                            curr_pos.y -= font_size;
                        }
                    }

                    glm::vec2 offset = font_size * fc.offset;
                    glm::vec2 rect_pos = curr_pos + offset;
                    rect_pos.x += lb;
                    rect_pos.y -= rect_dim.y;

                    RenderRect2D & rect = data[data_curr];
                    rect.color = color;
                    rect.uv0 = o + glm::vec2{ 0.0f, dim.y };
                    rect.uv1 = o + glm::vec2{ dim.x, dim.y };
                    rect.uv2 = o + glm::vec2{ dim.x, 0.0f };
                    rect.uv3 = o;
                    /* convert to view space coordinates */
                    rect.position = 2.0f * (rect_pos / glm::vec2{w, h}) - 1.0f;
                    rect.dimension = 2.0f * (rect_dim / glm::vec2{w, h});
                    rect.texture = n.texture;
                    rect.layer = rect_layer;

                    ++data_curr;

                    curr_pos.x += advance;
                }
                break;
            }
            case CanvasNode::Mode::invisible: {
                break;
            }
        }

        stack_info.push_back(info);
    }
}
