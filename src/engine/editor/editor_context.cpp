#include "editor_context.h"

#include "src/engine/editor/editor.h"

#include <sstream>

using namespace prt3;

EditorContext::EditorContext(Editor & editor, Context & context)
 : m_editor{editor},
   m_context{context}
{
}

void EditorContext::commit_frame() {
    m_context.edit_scene().update_transform_cache();
}
