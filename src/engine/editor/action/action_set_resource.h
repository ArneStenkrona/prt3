#ifndef PRT3_ACTION_SET_RESOURCE_H
#define PRT3_ACTION_SET_RESOURCE_H

#include "src/engine/scene/scene.h"
#include "src/engine/editor/editor_context.h"

#include "src/engine/editor/action/action.h"

namespace prt3 {

template<typename ComponentType>
class ActionSetResource : public Action {
public:
    ActionSetResource(
        EditorContext & editor_context,
        NodeID node_id,
        ResourceID resource_id
    ) : m_editor_context{&editor_context},
        m_node_id{node_id},
        m_resource_id{resource_id}
    {
        Scene const & scene = m_editor_context->scene();

        m_original_resource_id =
            scene.get_component<ComponentType>(m_node_id).resource_id();
    }

protected:
    virtual bool apply() {
        Scene & scene = m_editor_context->scene();
        scene.get_component<ComponentType>(m_node_id)
            .set_resource_id(m_resource_id);
        return true;
    }

    virtual bool unapply() {
        Scene & scene = m_editor_context->scene();
        scene.get_component<ComponentType>(m_node_id)
            .set_resource_id(m_original_resource_id);
        return true;
    }

private:
    EditorContext * m_editor_context;
    NodeID m_node_id;

    ResourceID m_resource_id;
    ResourceID m_original_resource_id;
};

} // namespace prt3

#endif
