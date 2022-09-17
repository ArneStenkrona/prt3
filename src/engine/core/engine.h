#ifndef PRT3_ENGINE_H
#define PRT3_ENGINE_H

#include "src/engine/core/context.h"

#include <array>
#include <cstdint>
#include <chrono>

using time_point = std::chrono::time_point<std::chrono::high_resolution_clock>;

namespace prt3
{

class Engine {
public:
    Engine();
    void start();

    void execute_frame();
private:
    void measure_duration();

    Context m_context;
    uint64_t m_frame_number = 0;

    bool m_print_framerate = false;
    std::array<int64_t, 10> m_frame_duration_buffer;
    time_point m_last_frame_time_point;

};

}

#endif
