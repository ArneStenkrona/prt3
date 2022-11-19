#ifndef PRT3_NODE_TRANSFORM_ACTION_H
#define PRT3_NODE_TRANSFORM_ACTION_H

#include "src/engine/editor/action/action.h"
#include "src/engine/scene/node.h"
#include "src/engine/component/transform.h"

namespace prt3 {

class NodeTransformAction : public Action {
public:
    NodeTransformAction(
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
