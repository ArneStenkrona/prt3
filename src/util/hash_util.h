#ifndef PRT3_HASH_UTIL_H
#define PRT3_HASH_UTIL_H

#include <functional>

namespace prt3 {

template <class T>
inline void hash_combine(std::size_t & s, const T & v)
{
    std::hash<T> h;
    s^= h(v) + 0x9e3779b9 + (s<< 6) + (s>> 2);
}

} // namespace prt3

#endif
