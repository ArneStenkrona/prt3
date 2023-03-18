#ifndef PRT3_MATH_UTIL_H
#define PRT3_MATH_UTIL_H

#include <cstdint>

namespace prt3 {

    inline uint8_t number_of_set_bits(uint32_t u) {
        u = u - ((u >> 1) & 0x55555555);
        u = (u & 0x33333333) + ((u >> 2) & 0x33333333);
        return (((u + (u >> 4)) & 0x0F0F0F0F) * 0x01010101) >> 24;
    }

    inline uint8_t number_of_set_bits(uint64_t u) {
        u = u - ((u >> 1) & 0x5555555555555555);
        u = (u & 0x3333333333333333) + ((u >> 2) & 0x3333333333333333);
        return (((u + (u >> 4)) & 0x0F0F0F0F0F0F0F0F) *
                0x0101010101010101) >> 56;
    }

    inline void set_nth_bit(uint8_t * ptr, size_t n) {
        ptr[n / 8] = ptr[n / 8] | (1 << n % 8);
    }

    inline void clear_nth_bit(uint8_t * ptr, size_t n) {
        ptr[n / 8] = ptr[n / 8] & ~(1 << n % 8);
    }

} // namespace prt3

#endif // PRT3_MATH_UTIL_H
