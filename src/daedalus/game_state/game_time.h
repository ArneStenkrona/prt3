#ifndef DDS_GAME_TIME_H
#define DDS_GAME_TIME_H

#include <cstdint>

namespace dds {

typedef int64_t TimeMS; // in-game time in milliseconds
constexpr TimeMS time_scale = 72;
constexpr TimeMS ms_per_frame = (time_scale * 10000) / 60;

} // namespace dds

#endif // DDS_GAME_TIME_H
