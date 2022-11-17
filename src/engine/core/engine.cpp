#include "engine.h"

#include <iostream>
#include <algorithm>

using namespace prt3;

using time_point = std::chrono::time_point<std::chrono::high_resolution_clock>;

Engine::Engine()
 : m_editor{m_context},
   m_last_frame_time_point{std::chrono::high_resolution_clock::now()} {

}

void Engine::execute_frame() {
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



    static RenderData render_data;
    render_data.clear();
    switch (m_mode) {
        case EngineMode::game: {
            Scene & scene = m_context.game_scene();

            scene.update(fixed_delta_time);

            scene.collect_world_render_data(
                render_data.world,
                m_editor.selected_node()
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

            m_context.renderer().render(render_data, true);
            break;
        }
    }

    // loop end
    measure_duration();

    ++m_frame_number;
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
        std::cout << "framerate: " << fps
                  << " fps (" << avg_ms << " ms)" << std::endl;
    }

    m_last_frame_time_point = now;
}

void Engine::set_mode_game() {
    m_mode = EngineMode::game;
    m_context.set_game_scene(m_context.edit_scene());
}

void Engine::set_mode_editor() {
    m_mode = EngineMode::editor;
}

