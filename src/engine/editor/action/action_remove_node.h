#ifndef PRT3_ACTION_REMOVE_NODE_H
#define PRT3_ACTION_REMOVE_NODE_H

#include "src/engine/editor/action/action.h"
#include "src/engine/scene/node.h"

#include <vector>
#include <unordered_map>

namespace prt3 {

class ActionRemoveNode : public Action {
public:
    ActionRemoveNode(
        EditorContext & editor_context,
        NodeID node_id
    );

protected:
    virtual bool apply();
    virtual bool unapply();

private:
    EditorContext * m_editor_context;
    NodeID m_node_id;

    std::vector<char> m_data;

    void serialize_node(std::ostream & out) const;
    void deserialize_node(std::istream & in);
};

} // namespace prt3

#endif
