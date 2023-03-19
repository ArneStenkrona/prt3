#ifndef PRT3_ENGINE_H
#define PRT3_ENGINE_H

#include "src/engine/core/context.h"
#include "src/engine/editor/editor.h"

#include <array>
#include <cstdint>
#include <chrono>

using time_point = std::chrono::time_point<std::chrono::high_resolution_clock>;

namespace prt3 {

enum class EngineMode {
    game,
    editor
};

class Engine {
public:
    Engine();

    void set_project_from_path(std::string const & path);

    bool execute_frame();
private:
    void measure_duration();

    void set_mode_game();
    void set_mode_editor();

    Context m_context;
    Editor m_editor;
    uint64_t m_frame_number = 0;
    EngineMode m_mode = EngineMode::editor;

    bool m_print_framerate = false;
    std::array<int64_t, 10> m_frame_duration_buffer;
    time_point m_last_frame_time_point;

};

}

#endif
