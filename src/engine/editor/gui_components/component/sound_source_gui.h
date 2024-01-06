#ifndef PRT3_SOUND_SOURCE_GUI_H
#define PRT3_SOUND_SOURCE_GUI_H

#include "src/engine/editor/gui_components/component/component_gui.h"
#include "src/engine/editor/gui_components/component/component_gui_utility.h"
#include "src/engine/editor/editor_context.h"
#include "src/engine/component/sound_source.h"

#include "imgui.h"

namespace prt3 {

template<>
void inner_show_component<SoundSourceComponent>(
    EditorContext & context,
    NodeID id
) {
    ImGui::PushItemWidth(160.0f);

    edit_field<SoundSourceComponent, float>(
        context,
        id,
        "pitch",
        offsetof(SoundSourceComponent, m_pitch)
    );

    edit_field<SoundSourceComponent, float>(
        context,
        id,
        "gain",
        offsetof(SoundSourceComponent, m_gain)
    );

    edit_field<SoundSourceComponent, float>(
        context,
        id,
        "rolloff factor",
        offsetof(SoundSourceComponent, m_rolloff_factor)
    );

    edit_field<SoundSourceComponent, float>(
        context,
        id,
        "reference distance",
        offsetof(SoundSourceComponent, m_reference_distance)
    );

    edit_field<SoundSourceComponent, float>(
        context,
        id,
        "max distance",
        offsetof(SoundSourceComponent, m_max_distance)
    );

    ImGui::PopItemWidth();
}

}

#endif // PRT3_SOUND_SOURCE_GUI_H
