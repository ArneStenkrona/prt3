#ifndef PRT3_MEM_H
#define PRT3_MEM_H

#include <istream>
#include <streambuf>

struct membuf: std::streambuf {
    membuf(char const * base, size_t size) {
        char * p(const_cast<char*>(base));
        this->setg(p, p, p + size);
    }
};

struct imemstream: virtual membuf, std::istream {
    imemstream(char const * base, size_t size)
        : membuf(base, size)
        , std::istream(static_cast<std::streambuf*>(this)) {
    }
};

#endif
