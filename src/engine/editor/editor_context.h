#ifndef PRT3_EDITOR_CONTEXT_H
#define PRT3_EDITOR_CONTEXT_H

#include "src/engine/core/context.h"
#include "src/engine/scene/node.h"

#include <vector>

namespace prt3 {

class Editor;

class EditorContext {
public:
    EditorContext(Editor & editor, Context & context);

    Editor & editor() { return m_editor; }
    Context & context() { return m_context; }

    std::vector<Node> & get_scene_nodes() { return context().scene().m_nodes; }

    void set_selected_node(NodeID id) { m_selected_node = id; }
private:
    Editor & m_editor;
    Context & m_context;

    NodeID m_selected_node = NO_NODE;
};

} // namespace prt3

#endif
