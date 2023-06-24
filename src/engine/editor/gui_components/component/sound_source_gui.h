#ifndef PRT3_SOUND_SOURCE_GUI_H
#define PRT3_SOUND_SOURCE_GUI_H

#include "src/engine/editor/gui_components/component/component_gui.h"
#include "src/engine/editor/gui_components/component/component_gui_utility.h"
#include "src/engine/editor/editor_context.h"
#include "src/engine/component/sound_source.h"

#include "imgui.h"

namespace prt3 {

class ActionSetSoundSource : public Action {
public:
    ActionSetSoundSource(
        EditorContext & editor_context,
        NodeID node_id,
        float pitch,
        float gain
    ) : m_editor_context{&editor_context},
        m_node_id{node_id},
        m_pitch{pitch},
        m_gain{gain}
    {
        Scene const & scene = m_editor_context->scene();

        SoundSourceComponent const & comp =
            scene.get_component<SoundSourceComponent>(m_node_id);

        m_original_pitch = comp.pitch();
        m_original_gain = comp.gain();
    }

protected:
    virtual bool apply() {
        Scene & scene = m_editor_context->scene();

        SoundSourceComponent & comp =
            scene.get_component<SoundSourceComponent>(m_node_id);

        comp.set_pitch(scene, m_pitch);
        comp.set_gain(scene, m_gain);

        return true;
    }

    virtual bool unapply() {
        Scene & scene = m_editor_context->scene();

        SoundSourceComponent & comp =
            scene.get_component<SoundSourceComponent>(m_node_id);

        comp.set_pitch(scene, m_original_pitch);
        comp.set_gain(scene, m_original_gain);

        return true;
    }

private:
    EditorContext * m_editor_context;
    NodeID m_node_id;

    float m_pitch;
    float m_gain;

    float m_original_pitch;
    float m_original_gain;
};

template<>
void inner_show_component<SoundSourceComponent>(
    EditorContext & context,
    NodeID id
) {
    Scene const & scene = context.scene();

    SoundSourceComponent const & comp =
        scene.get_component<SoundSourceComponent>(id);

    float pitch = comp.pitch();
    float gain = comp.gain();

    bool changed = false;

    ImGui::PushItemWidth(160);
    changed |= display_value("pitch", pitch);
    changed |= display_value("gain", gain);
    ImGui::PopItemWidth();

    if (changed) {
        context.editor()
        .perform_action<ActionSetSoundSource>(
            id,
            pitch,
            gain
        );
    }
}

}

#endif // PRT3_SOUND_SOURCE_GUI_H
