#ifndef PRT3_SERIALIZATION_UTIL_H
#define PRT3_SERIALIZATION_UTIL_H

#include <iostream>
#include <cstring>
#include <cassert>

namespace prt3 {

template<typename T>
inline void write_stream(std::ostream & out, T const & val) {
    out.write(reinterpret_cast<char const *>(&val), sizeof(val));
}

template<typename T>
inline void read_stream(std::istream & in, T & val) {
    in.read(reinterpret_cast<char*>(&val), sizeof(val));
}

inline void write_string(std::ostream & out, std::string const & str) {
    write_stream(out, str.length());
    out.write(str.data(), str.length());
}

inline void read_string(std::istream & in, std::string & str) {
    size_t len;
    read_stream(in, len);
    str.resize(len);
    in.read(str.data(), len);
}

inline void write_c_string(std::ostream & out, char const * s) {
    size_t len = strlen(s);
    write_stream(out, len);
    out.write(s, len);
}

inline void read_c_string(std::istream & in, char * s, size_t max_len) {
    size_t len;
    read_stream(in, len);
    assert(len > max_len && "string can't fit in buffer");
    size_t n = len < max_len ? len : max_len;
    in.read(s, n);
    in.seekg(len - n, std::ios_base::cur);
}

} // namespace prt3

#endif
