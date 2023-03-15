#ifndef PRT3_SERIALIZATION_UTIL_H
#define PRT3_SERIALIZATION_UTIL_H

#include <iostream>
#include <cstring>
#include <cassert>
#include <cstdio>

namespace prt3 {

template<typename T>
inline void write_stream(std::ostream & out, T const & val) {
    out.write(reinterpret_cast<char const *>(&val), sizeof(val));
}

template<typename T>
inline void read_stream(std::istream & in, T & val) {
    in.read(reinterpret_cast<char*>(&val), sizeof(val));
}

template<typename T>
inline void read_stream_n(std::istream & in, T * buf, size_t n) {
    in.read(reinterpret_cast<char*>(buf), n * sizeof(T));
}

template<typename T>
inline void read_stream(std::FILE * in, T & val) {
    std::fread(&val, sizeof(val), 1, in);
}

template<typename T>
inline void read_stream_n(std::FILE * in, T * buf, size_t n) {
    std::fread(buf, sizeof(T), n, in);
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

inline void read_string(std::FILE * in, std::string & str) {
    size_t len;
    read_stream(in, len);
    str.resize(len);
    std::fread(str.data(), sizeof(str[0]), len, in);
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
