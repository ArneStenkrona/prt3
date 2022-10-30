#include "editor_context.h"

#include "src/engine/editor/editor.h"

using namespace prt3;

EditorContext::EditorContext(Editor & editor, Context & context)
 : m_editor{editor}, m_context{context} {

}

void EditorContext::commit_frame() {
    if (m_stale_transform_cache) {
        m_context.scene().update_transform_cache();
    }
    m_stale_transform_cache = false;
}