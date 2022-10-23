#include "editor_context.h"

#include "src/engine/editor/editor.h"

using namespace prt3;

EditorContext::EditorContext(Editor & editor, Context & context)
 : m_editor{editor}, m_context{context} {

}
