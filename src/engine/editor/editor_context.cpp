#include "editor_context.h"

#include "src/engine/editor/editor.h"

#include <sstream>

using namespace prt3;

EditorContext::EditorContext(Editor & editor, Context & context)
 : m_editor{editor},
   m_context{context}
{
}

  void EditorContext::set_save_point()
  { m_save_point = m_editor.action_manager().get_action_count(); }


  bool EditorContext::is_saved() const
  { return m_save_point == m_editor.action_manager().get_action_count(); }