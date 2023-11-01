#ifndef PRT3_CANVAS_H
#define PRT3_CANVAS_H

#include "src/engine/scene/node.h"
#include "src/engine/rendering/render_data.h"
#include "src/engine/rendering/resources.h"
#include "src/util/uuid.h"

#include <vector>

namespace prt3 {

class Scene;

template<typename T>
class ComponentStorage;

class EditorContext;

struct CanvasNode {
    enum class AnchorPoint {
        top_left,
        top,
        top_right,
        mid_left,
        mid,
        mid_right,
        bottom_left,
        bottom,
        bottom_right
    };

    enum class UnitType {
        absolute, // pixel coordinates
        relative // percentage of parent dimensions
    };

    /* dimensions */
    glm::vec2 dimension;
    UnitType dimension_mode;

    /* position relative to parent */
    glm::vec2 position;
    glm::vec2 origin;
    UnitType position_mode;
    UnitType origin_mode;
    AnchorPoint parent_anchor;
    AnchorPoint center_point;

    bool rendered; // should the node be rendered?
    glm::vec4 color;
    bool inherit_color; // whether the color should be blended with ancestors

    /* texture info, ignored if texture resource id is -1 */
    glm::vec2 uv0;
    glm::vec2 uv1;
    glm::vec2 uv2;
    glm::vec2 uv3;
    ResourceID texture;

    int16_t layer; // render layer, relative to parent, higher occludes lower

};

struct CanvasStackNode {
    CanvasNode n;
    int32_t parent;
};

class Canvas {
public:
    Canvas(Scene & scene, NodeID node_id);
    Canvas(Scene & scene, NodeID node_id, std::istream & in);

    NodeID node_id() const { return m_node_id; }

    void serialize(
        std::ostream & out,
        Scene const & scene
    ) const;

    static char const * name() { return "Canvas"; }
    static constexpr UUID uuid = 15829914204448507677ull;

    void collect_render_data(
        Scene const & scene,
        std::vector<RenderRect2D> & data
    );

    void begin_node(CanvasNode const & node) {
        CanvasStackNode n;
        n.n = node;
        n.parent = m_current_parent;
        m_current_parent = m_node_stack.size();
        m_node_stack.push_back(n);
    }

    void end_node() {
        m_current_parent = m_node_stack[m_current_parent].parent;
    }

    int16_t & layer() { return m_layer; }

private:
    NodeID m_node_id;
    std::vector<CanvasStackNode> m_node_stack;
    int32_t m_current_parent = -1;

    int16_t m_layer;

    void reset_stack() {
        m_node_stack.clear();
        m_current_parent = -1;
    }

    void remove(Scene & /*scene*/) {}

    friend class ComponentStorage<Canvas>;
    friend class Scene;
};

} // namespace prt3

#endif // PRT3_CANVAS_H
