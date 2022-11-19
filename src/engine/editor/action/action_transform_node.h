#ifndef PRT3_ACTION_TRANSFORM_NODE_H
#define PRT3_ACTION_TRANSFORM_NODE_H

#include "src/engine/editor/action/action.h"
#include "src/engine/scene/node.h"
#include "src/engine/component/transform.h"

namespace prt3 {

class ActionTransformNode : public Action {
public:
    ActionTransformNode(
        EditorContext & editor_context,
        NodeID node_id,
        Transform const & original_transform,
        Transform const & new_transform
    );

    virtual bool apply();
    virtual bool unapply();

private:
    EditorContext * m_editor_context;

    NodeID m_node_id;
    Transform m_original_transform;
    Transform m_new_transform;
};

} // namespace prt3

#endif
