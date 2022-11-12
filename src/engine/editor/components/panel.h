#ifndef PRT3_PANEL_H
#define PRT3_PANEL_H

#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui.h"
#include "imgui_internal.h"

namespace prt3 {

void begin_group_panel(
    char const * name,
    ImVec2 const & size = ImVec2(-1.0f, -1.0f)
);

void end_group_panel();

bool begin_group_panel_with_button(
    char const * name,
    char const * button_text,
    ImVec2 const & size = ImVec2(-1.0f, -1.0f)
);

} // namespace prt3

#endif
