#ifndef PRT3_SERIALIZATION_UTIL_H
#define PRT3_SERIALIZATION_UTIL_H

#include <iostream>

namespace prt3 {

template<typename T>
inline void write_stream(std::ostream & out, T const & val) {
    out.write(reinterpret_cast<char const *>(&val), sizeof(val));
}

template<typename T>
inline void read_stream(std::istream & in, T & val) {
    in.read(reinterpret_cast<char*>(&val), sizeof(val));
}

} // namespace prt3

#endif
