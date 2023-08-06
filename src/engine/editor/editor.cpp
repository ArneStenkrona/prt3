#include "editor.h"

#include "src/engine/editor/editor_context.h"
#include "src/engine/editor/gui_components/editor_gui.h"
#include "src/engine/scene/node.h"
#include "src/util/mesh_util.h"

#include "imgui.h"

#include <array>

using namespace prt3;

Editor::Editor(Context & context)
 : m_context{context},
   m_camera{context.renderer().window_width(),
            context.renderer().window_height()},
   m_editor_context{*this, m_context},
   m_gizmo_manager{m_editor_context, m_camera.get_camera()},
   m_action_manager{m_editor_context} {
    std::array<glm::vec3, 24> box_verts;
    insert_line_box(glm::vec3{-0.5f}, glm::vec3{0.5f}, box_verts.data());

    m_line_box_res_id = m_context.renderer().upload_pos_mesh(
        box_verts.data(),
        box_verts.size()
    );
}

Editor::~Editor() {
        m_context.renderer().free_pos_mesh(m_line_box_res_id);
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

    if (input.get_key(KEY_CODE_LEFT_CONTROL)){
        if (input.get_key_down(KEY_CODE_Z)) {
            if (input.get_key(KEY_CODE_LEFT_SHIFT)) {
                m_action_manager.redo();
            } else {
                m_action_manager.undo();
            }
        }
    }

    ImGui::Render();
}

void Editor::collect_render_data(
    EditorRenderData & data
) {
    PhysicsSystem & physics_sys = m_context.edit_scene().physics_system();
    NavigationSystem & nav_sys = m_context.edit_scene().navigation_system();
    Renderer & renderer = m_context.renderer();
    auto const & transforms =
        m_context.edit_scene().m_transform_cache.global_transforms();

    physics_sys.collect_render_data(
        renderer,
        transforms.data(),
        m_editor_context.get_selected_node(),
        data
    );

    nav_sys.collect_render_data(
        renderer,
        m_editor_context.get_selected_node(),
        data
    );

    auto & armatures =
        m_context.edit_scene()
        .m_component_manager
        .get_all_components<Armature>();

    for (Armature & armature : armatures) {
        auto flags =
            m_context.edit_scene().get_node_mod_flags(armature.node_id());
        if (flags | Node::ModFlags::mod_flag_descendant_removed ||
            flags | Node::ModFlags::mod_flag_descendant_added) {
            armature.map_bones(m_context.edit_scene());
        }
    }

    Scene const & scene = m_context.edit_scene();
    if (scene.has_component<Decal>(
        m_editor_context.get_selected_node()
    )) {
        Decal const & decal = scene.get_component<Decal>(
            m_editor_context.get_selected_node()
        );
        Transform tform =
            scene.get_node(decal.node_id()).get_global_transform(scene);
        tform.scale *= decal.dimensions();

        WireframeRenderData decal_data;
        decal_data.mesh_id = m_line_box_res_id;
        decal_data.color = glm::vec4{0.0f, 1.0f, 0.0f, 1.0f};
        decal_data.transform = tform.to_matrix();

        data.line_data.push_back(decal_data);
    }

    m_context.edit_scene().clear_node_mod_flags();
}
