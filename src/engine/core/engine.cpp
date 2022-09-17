#include "engine.h"

#include <iostream>
#include <algorithm>

using namespace prt3;

using time_point = std::chrono::time_point<std::chrono::high_resolution_clock>;

Engine::Engine()
 : m_last_frame_time_point{std::chrono::high_resolution_clock::now()} {

}

void Engine::execute_frame() {
    // loop begin
    float fixed_delta_time = 1.0f / 60.0f;
    m_context.input().update();
    m_context.current_scene().update(fixed_delta_time);
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

    if (m_context.input().get_key_down(KEY_CODE_PERIOD)) {
        m_print_framerate = !m_print_framerate;
    }

    if (m_print_framerate && m_frame_number % 10 == 0) {
        std::cout << "framerate: " << fps
                  << " fps (" << avg_ms << " ms)" << std::endl;
    }

    m_last_frame_time_point = now;
}
