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
            return glm::vec2{0.0f, 1.0f};
        }
        case CanvasNode::AnchorPoint::top: {
            return glm::vec2{0.5f, 1.0f};
        }
        case CanvasNode::AnchorPoint::top_right: {
            return glm::vec2{1.0f, 1.0f};
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
            return glm::vec2{0.0f, 0.0f};
        }
        case CanvasNode::AnchorPoint::bottom: {
            return glm::vec2{0.5f, 0.0f};
        }
        case CanvasNode::AnchorPoint::bottom_right: {
            return glm::vec2{1.0f, 0.0f};
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
            *reinterpret_cast<unsigned const char *>(&str[index]);
        width += text.font_size * text.char_info[c].advance;
        ++index;
    }

    return width;
}

static void align_text_x(
    RenderRect2D * rects,
    uint32_t row_start,
    uint32_t row_end,
    float screen_width,
    float parent_width,
    float row_width,
    prt3::CanvasNode::AnchorPoint alignment
) {
    float adjustment;
    switch (alignment) {
        case prt3::CanvasNode::AnchorPoint::top_left:
        case prt3::CanvasNode::AnchorPoint::mid_left:
        case prt3::CanvasNode::AnchorPoint::bottom_left: {
            adjustment = -parent_width / screen_width;
            break;
        }
        case prt3::CanvasNode::AnchorPoint::top:
        case prt3::CanvasNode::AnchorPoint::mid:
        case prt3::CanvasNode::AnchorPoint::bottom: {
            adjustment = (parent_width - row_width) / screen_width;
            break;
        }
        case prt3::CanvasNode::AnchorPoint::top_right:
        case prt3::CanvasNode::AnchorPoint::mid_right:
        case prt3::CanvasNode::AnchorPoint::bottom_right: {
            adjustment = (parent_width + 2.0f * (parent_width - row_width)) /
                         screen_width;
            break;
        }
    }

    for (uint32_t i = row_start; i < row_end; ++i) {
        RenderRect2D & rect = rects[i];
        rect.position.x += adjustment;
    }
}

static void align_text_y(
    RenderRect2D * rects,
    uint32_t text_start,
    uint32_t text_end,
    float screen_height,
    float parent_height,
    float text_height,
    float font_size,
    prt3::CanvasNode::AnchorPoint alignment
) {
    float adjustment;
    switch (alignment) {
        case prt3::CanvasNode::AnchorPoint::top_left:
        case prt3::CanvasNode::AnchorPoint::top:
        case prt3::CanvasNode::AnchorPoint::top_right: {
            adjustment = -parent_height / screen_height;
            break;
        }
        case prt3::CanvasNode::AnchorPoint::mid_left:
        case prt3::CanvasNode::AnchorPoint::mid:
        case prt3::CanvasNode::AnchorPoint::mid_right: {
            adjustment = -(parent_height - text_height - 1.5f * font_size) /
                           screen_height;
            break;
        }
        case prt3::CanvasNode::AnchorPoint::bottom_left:
        case prt3::CanvasNode::AnchorPoint::bottom:
        case prt3::CanvasNode::AnchorPoint::bottom_right: {
            adjustment = (parent_height -
                         2.0f * (parent_height - text_height - font_size)) /
                         screen_height;
            break;
        }
    }

    for (uint32_t i = text_start; i < text_end; ++i) {
        RenderRect2D & rect = rects[i];
        rect.position.y += adjustment;
    }
}

void Canvas::collect_render_data(
    Scene const & scene,
    std::vector<RenderRect2D> & data
) const {
    struct StackInfo {
        glm::vec4 color;
        glm::vec2 dimension;
        glm::vec2 position;
        int32_t layer;
        int32_t index;
    };

    thread_local std::vector<StackInfo> stack_info;
    stack_info.resize(1);
    stack_info[0].color = glm::vec4{1.0f, 1.0f, 1.0f, 1.0f};
    stack_info[0].position = glm::vec2{0.0f};
    stack_info[0].layer = 0;
    stack_info[0].index = -1;

    unsigned int w, h;
    scene.get_window_size(w, h);
    stack_info[0].dimension = glm::vec2{w, h};

    int32_t index = 0;
    for (CanvasStackNode const & sn : m_node_stack) {
        CanvasNode const & n = sn.n;

        while (stack_info.back().index != sn.parent) {
            stack_info.pop_back();
        }

        StackInfo const & curr_info = stack_info.back();
        StackInfo info;
        info.index = index;

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

        glm::vec2 parent_anchor = get_anchor_factor(n.parent_anchor);

        /* position */
        glm::vec2 position =
            curr_info.position +
            parent_anchor * curr_info.dimension;

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

        glm::vec2 center_anchor = get_anchor_factor(n.center_point);
        position += -center_anchor * dimension;

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

                data.emplace_back(rect);

                break;
            }
            case CanvasNode::Mode::text: {
                CanvasText const & text = n.u.text;
                float font_size = static_cast<float>(text.font_size);

                position.x += (0.5f - center_anchor.x) * dimension.x;
                position.y += (0.5f + center_anchor.y) * dimension.y - font_size;

                glm::vec2 curr_pos = position;
                float start_y = curr_pos.y;

                float row_width = 0.0f;
                uint32_t text_rect_start = data.size();
                uint32_t row_start = text_rect_start;

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
                        if (row_width + curr_word_width > dimension.x) {
                            /* reposition previous row */
                            align_text_x(
                                data.data(),
                                row_start,
                                i,
                                w,
                                dimension.x,
                                row_width,
                                n.center_point
                            );

                            curr_pos.x = position.x;
                            curr_pos.y -= font_size;
                            row_start = i;
                            row_width = 0.0f;
                        }
                    }

                    glm::vec2 offset = font_size * fc.offset;
                    glm::vec2 rect_pos = curr_pos + offset;
                    rect_pos.x += lb;
                    rect_pos.y -= rect_dim.y;

                    RenderRect2D rect;
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

                    data.emplace_back(rect);

                    curr_pos.x += advance;
                    row_width += advance;
                }

                /* align last row */
                align_text_x(
                    data.data(),
                    row_start,
                    data.size(),
                    w,
                    dimension.x,
                    row_width,
                    n.center_point
                );

                float text_height = start_y - curr_pos.y;
                align_text_y(
                    data.data(),
                    text_rect_start,
                    data.size(),
                    h,
                    dimension.y,
                    text_height,
                    text.font_size,
                    n.center_point
                );

                break;
            }
            case CanvasNode::Mode::invisible: {
                break;
            }
        }

        stack_info.push_back(info);
        ++index;
    }
}
