#ifndef PRT3_ACTION_ADD_COMPONENT_H
#define PRT3_ACTION_ADD_COMPONENT_H

#include "src/engine/editor/action/action.h"

#include "src/engine/scene/scene.h"
#include "src/engine/editor/editor_context.h"

namespace prt3 {

template<typename ComponentType>
class ActionAddComponent : public Action {
public:
    ActionAddComponent(
        EditorContext & editor_context,
        NodeID node_id
    ) : m_editor_context{&editor_context},
        m_node_id{node_id} {}

protected:
    virtual bool apply() {
        Scene & scene = m_editor_context->scene();
        scene.add_component<ComponentType>(m_node_id);
        return true;
    }

    virtual bool unapply() {
        Scene & scene = m_editor_context->scene();
        return scene.remove_component<ComponentType>(m_node_id);
    }

private:
    EditorContext * m_editor_context;
    NodeID m_node_id;
};

} // namespace prt3

#endif
