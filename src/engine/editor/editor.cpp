#include "editor.h"

#include "src/engine/editor/editor_context.h"
#include "src/engine/editor/gui_components/editor_gui.h"
#include "src/engine/scene/node.h"

#include "imgui.h"

#include <iostream>

using namespace prt3;

Editor::Editor(Context & context)
 : m_context{context},
   m_camera{context.renderer().window_width(),
            context.renderer().window_height()},
   m_editor_context{*this, m_context},
   m_gizmo_manager{m_editor_context, m_camera.get_camera()},
   m_action_manager{m_editor_context} {
}

void Editor::update(float delta_time) {
    Input const & input = m_context.input();

    ImGui::NewFrame();

    editor_gui(m_editor_context);
    m_gizmo_manager.update();

    if (!ImGui::IsWindowFocused(ImGuiFocusedFlags_AnyWindow) &&
        !ImGuizmo::IsUsing()) {
        m_camera.update(delta_time, input);

        if (input.get_key_down(KEY_CODE_MOUSE_RIGHT)) {
            int x, y;
            input.get_cursor_position(x, y);

            NodeID id = m_context.renderer().get_selected(x, y);
            m_editor_context.set_selected_node(id);
        }
    }

    if (input.get_key(KEY_CODE_LCTRL)){
        if (input.get_key_down(KEY_CODE_Z)) {
            if (input.get_key(KEY_CODE_LSHIFT)) {
                m_action_manager.redo();
            } else {
                m_action_manager.undo();
            }
        }
    }

    ImGui::Render();
}

void Editor::collect_collider_render_data(
    EditorRenderData & data
) {
    PhysicsSystem & sys = m_context.edit_scene().physics_system();
    Renderer & renderer = m_context.renderer();
    auto const & transforms =
        m_context.edit_scene().m_transform_cache.global_transforms();

    sys.collect_collider_render_data(
        renderer,
        transforms.data(),
        m_editor_context.get_selected_node(),
        data.collider_data
    );

    auto & armatures =
        m_context.edit_scene()
        .m_component_manager
        .get_all_components<Armature>();

    for (Armature & armature : armatures) {
        armature.validate_and_map_node_children(m_context.edit_scene());
    }
    m_context.edit_scene().clear_node_mod_flags();
}
