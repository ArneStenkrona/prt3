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
            color = n.color;
        } else {
            color = curr_info.color * n.color;
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

        if (n.rendered) {
            RenderRect2D rect;
            rect.color = color;
            rect.uv0 = n.uv0;
            rect.uv1 = n.uv1;
            rect.uv2 = n.uv2;
            rect.uv3 = n.uv3;
            /* convert to view space coordinates */
            rect.position = 2.0f * (position / glm::vec2{w, h}) - 1.0f;
            rect.dimension = 2.0f * (dimension / glm::vec2{w, h});
            rect.texture = n.texture;
            rect.layer = n.layer + m_layer * (UINT16_MAX + 1);

            data.push_back(rect);
        }

        stack_info.push_back(info);
    }
}
