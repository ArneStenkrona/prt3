#ifndef PRT3_EDITOR_H
#define PRT3_EDITOR_H

#include "src/engine/core/context.h"
#include "src/engine/editor/editor_camera.h"
#include "src/engine/editor/editor_context.h"

namespace prt3 {

class Editor {
public:
    Editor(Context & context);

    void update(float delta_time);

    Camera & get_camera() { return m_camera.get_camera(); }
private:
    Context & m_context;
    EditorCamera m_camera;

    EditorContext m_editor_context;
};

};

#endif
