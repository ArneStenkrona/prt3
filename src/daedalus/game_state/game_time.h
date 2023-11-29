#ifndef DDS_GAME_TIME_H
#define DDS_GAME_TIME_H

#include <cstdint>

namespace dds {

typedef int64_t TimeMS; // in-game time in milliseconds
constexpr TimeMS time_scale = 72;
constexpr uint32_t assumed_framerate = 60;
constexpr TimeMS ms_per_frame = (time_scale * 1000) / assumed_framerate;
constexpr float frame_dt = float(time_scale) / float(assumed_framerate);
constexpr float frame_dt_over_time_scale = 1.0f / float(assumed_framerate);

} // namespace dds

#endif // DDS_GAME_TIME_H
