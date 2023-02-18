#ifndef PRT3_PROJECT_CONFIG_H
#define PRT3_PROJECT_CONFIG_H

#include "src/engine/editor/editor_context.h"

#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui.h"
#include "imgui_internal.h"

namespace prt3 {

void project_config(EditorContext & context, bool & open);

} // namespace prt3

#endif // PRT3_PROJECT_CONFIG_H
