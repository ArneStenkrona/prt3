#ifndef PRT3_PANEL_H
#define PRT3_PANEL_H

#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui.h"
#include "imgui_internal.h"

namespace prt3 {

void begin_group_panel(const char* name, const ImVec2& size = ImVec2(-1.0f, -1.0f));
void end_group_panel();

} // namespace prt3

#endif
