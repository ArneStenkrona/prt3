#ifndef DDS_GAME_TIME_H
#define DDS_GAME_TIME_H

#include <cstdint>

namespace dds {

typedef int64_t TimeMS; // in-game time in milliseconds
/* one day is 20 minutes irl at 60fps, i.e a 72X speed-up */
constexpr TimeMS ms_per_frame = (72 * 10000) / 60;

} // namespace dds

#endif // DDS_GAME_TIME_H
