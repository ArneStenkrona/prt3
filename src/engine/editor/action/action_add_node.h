#ifndef PRT3_ACTION_ADD_NODE_H
#define PRT3_ACTION_ADD_NODE_H

#include "src/engine/editor/action/action.h"
#include "src/engine/scene/node.h"

namespace prt3 {

class ActionAddNode : public Action {
public:
    ActionAddNode(
        EditorContext & editor_context,
        NodeName const & name,
        NodeID parent_id
    );

    virtual bool apply();
    virtual bool unapply();
private:
    EditorContext * m_editor_context;
    NodeName m_name;
    NodeID m_parent_id;
    NodeID m_node_id;
};

}

#endif
