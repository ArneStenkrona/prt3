#ifndef PRT3_EDITOR_CONTEXT_H
#define PRT3_EDITOR_CONTEXT_H

#include "src/engine/core/context.h"
#include "src/engine/scene/node.h"

#include "ImGuiFileBrowser.h"

#include <vector>

namespace prt3 {

class Editor;

class EditorContext {
public:
    EditorContext(Editor & editor, Context & context);

    Editor & editor() { return m_editor; }
    Context & context() { return m_context; }

    imgui_addons::ImGuiFileBrowser & file_dialog() { return m_file_dialog; }

    Scene & scene() { return context().edit_scene(); }
    Project & project() { return context().project(); }

    std::vector<Node> & get_scene_nodes()
    { return scene().m_nodes; }

    MaterialManager & get_material_manager()
    { return m_context.material_manager(); }

    ModelManager & get_model_manager()
    { return m_context.model_manager(); }

    NodeID get_selected_node() const { return m_selected_node; }
    void set_selected_node(NodeID id) { m_selected_node = id; }

private:
    Editor & m_editor;
    Context & m_context;

    imgui_addons::ImGuiFileBrowser m_file_dialog;

    NodeID m_selected_node = NO_NODE;
};

} // namespace prt3

#endif
