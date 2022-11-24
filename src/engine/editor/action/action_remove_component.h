#ifndef PRT3_ACTION_REMOVE_COMPONENT_H
#define PRT3_ACTION_REMOVE_COMPONENT_H

#include "src/engine/editor/action/action.h"

#include "src/engine/scene/scene.h"
#include "src/engine/editor/editor_context.h"
#include "src/util/mem.h"

#include <sstream>
#include <iostream>

namespace prt3 {

template<typename ComponentType>
class ActionRemoveComponent : public Action {
public:
    ActionRemoveComponent(
        EditorContext & editor_context,
        NodeID node_id
    ) : m_editor_context{&editor_context},
        m_node_id{node_id} {

        std::stringstream stream;

        Scene const & scene = m_editor_context->scene();

        scene.serialize_component<ComponentType>(stream, m_node_id);

        std::string const & s = stream.str();
        m_data.reserve(s.size());
        m_data.assign(s.begin(), s.end());
    }

protected:
    virtual bool apply() {
        Scene & scene = m_editor_context->scene();
        return scene.remove_component<ComponentType>(m_node_id);
    }

    virtual bool unapply() {
        imemstream in(m_data.data(), m_data.size());
        Scene & scene = m_editor_context->scene();
        scene.deserialize_component<ComponentType>(in, m_node_id);
        return true;
    }

private:
    EditorContext * m_editor_context;
    NodeID m_node_id;

    std::vector<char> m_data;
};

} // namespace prt3

#endif
