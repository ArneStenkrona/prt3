#include "engine.h"

#include "src/util/checksum.h"

#include <fstream>
#include <algorithm>

using namespace prt3;

using time_point = std::chrono::time_point<std::chrono::high_resolution_clock>;

Engine::Engine()
 : m_editor{m_context},
   m_last_frame_time_point{std::chrono::high_resolution_clock::now()} {
    set_mode_editor();
}

void Engine::set_project_from_path(std::string const & path) {
    m_context.set_project_from_path(path);
}

bool Engine::execute_frame() {
    // loop begin
    float fixed_delta_time = 1.0f / 60.0f;

    Input & input = m_context.input();
    m_context.input().update();

    EngineMode prev_mode = m_mode;

    if (input.get_key_down(KEY_CODE_TAB) &&
        input.get_key(KEY_CODE_LALT)) {
        switch (m_mode) {
            case EngineMode::game: {
                set_mode_editor();
                break;
            }
            case EngineMode::editor: {
                set_mode_game();
                break;
            }
        }
    }

    if (m_mode == EngineMode::editor &&
        prev_mode == EngineMode::editor) {
        m_context
            .renderer()
            .process_input_events(m_context.input().event_queue());
    }

    thread_local RenderData render_data;
    render_data.clear();
    switch (m_mode) {
        case EngineMode::game: {
            m_context.load_scene_if_queued();
            Scene & scene = m_context.game_scene();

            scene.update(fixed_delta_time);

            scene.collect_world_render_data(
                render_data.world,
                NO_NODE
            );

            scene.get_camera().collect_camera_render_data(
                render_data.camera_data
            );

            m_context.renderer().render(render_data, false);
            break;
        }
        case EngineMode::editor: {
            Scene & scene = m_context.edit_scene();

            m_context.renderer().prepare_imgui_rendering();

            m_editor.update(fixed_delta_time);

            scene.collect_world_render_data(
                render_data.world,
                m_editor.selected_node()
            );

            m_editor.get_camera().collect_camera_render_data(
                render_data.camera_data
            );

            m_editor.collect_render_data(render_data.editor_data);

            m_context.renderer().render(render_data, true);
            break;
        }
    }

    m_context.audio_manager().update();

    // loop end
    measure_duration();

    ++m_frame_number;

    return true;
}

void Engine::measure_duration() {
    auto now = std::chrono::high_resolution_clock::now();

    auto duration =
        std::chrono::duration_cast<std::chrono::microseconds>
        (now-m_last_frame_time_point);
    int64_t microseconds = duration.count();

    size_t n_buf = m_frame_duration_buffer.size();
    size_t index = m_frame_number % n_buf;
    m_frame_duration_buffer[index] = microseconds;

    int64_t sum = 0;
    int64_t n_avg =  std::min(m_frame_number, static_cast<uint64_t>(n_buf));
    for (size_t i = 0; i < n_avg; ++i) {
        sum += m_frame_duration_buffer[i];
    }
    int64_t avg_micro = sum / static_cast<int64_t>(n_buf);
    double avg_ms = avg_micro / 1000.0;
    double fps = 1000.0 / avg_ms;

    Input & input = m_context.input();
    if (input.get_key_down(KEY_CODE_PERIOD) &&
        input.get_key(KEY_CODE_LCTRL)) {
        m_print_framerate = !m_print_framerate;
    }

    if (m_print_framerate && m_frame_number % 10 == 0) {
        PRT3LOG("framerate: %f (%f ms)\n", fps, avg_ms);
    }

    m_last_frame_time_point = now;
}

void Engine::set_mode_game() {
    m_mode = EngineMode::game;
    m_context.set_game_scene(m_context.edit_scene());
    m_context.game_scene()
        .add_autoload_scripts(m_context.project().autoload_scripts());
    m_context.game_scene().start();
}

void Engine::set_mode_editor() {
    m_mode = EngineMode::editor;
    m_context.scene_manager().reset_queue();
}

