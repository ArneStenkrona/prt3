#include "scene_inspector.h"

#include "src/engine/editor/editor.h"
#include "src/engine/editor/action/action.h"
#include "src/engine/editor/gui_components/panel.h"

using namespace prt3;

class ActionSetAmbientLight : public Action {
public:
    ActionSetAmbientLight(
        EditorContext & editor_context,
        glm::vec3 color
    ) : m_editor_context{&editor_context},
        m_color{color}
    {
        Scene const & scene = editor_context.scene();
        m_original_color = scene.ambient_light();
    }

protected:
    virtual bool apply() {
        Scene & scene = m_editor_context->scene();
        scene.ambient_light() = m_color;
        return true;
    }

    virtual bool unapply() {
        Scene & scene = m_editor_context->scene();
        scene.ambient_light() = m_original_color;
        return true;
    }

private:
    EditorContext * m_editor_context;

    glm::vec3 m_color;
    glm::vec3 m_original_color;
};

bool show_ambient_light(glm::vec3 & light) {
    bool ret = false;

    begin_group_panel("ambient light");
    ImGui::PushItemWidth(160);

    ret = ImGui::ColorEdit3("color", &light.r);

    ImGui::PopItemWidth();

    end_group_panel();

    return ret;
}

void prt3::scene_inspector(EditorContext & context) {
    Scene & scene = context.scene();

    glm::vec3 light = scene.ambient_light();
    if (show_ambient_light(light)) {
        context.editor().perform_action<ActionSetAmbientLight>(light);
    }
}
