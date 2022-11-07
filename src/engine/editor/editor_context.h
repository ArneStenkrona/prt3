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

    void commit_frame();

    Editor & editor() { return m_editor; }
    Context & context() { return m_context; }

    Scene & scene() { return context().edit_scene(); }

    std::vector<Node> & get_scene_nodes()
    { return context().edit_scene().m_nodes; }

    NodeID get_selected_node() const { return m_selected_node; }
    void set_selected_node(NodeID id) { m_selected_node = id; }

    void invalidate_transform_cache() { m_stale_transform_cache = true; }
private:
    Editor & m_editor;
    Context & m_context;

    bool m_stale_transform_cache;

    NodeID m_selected_node = NO_NODE;
};

} // namespace prt3

#endif
