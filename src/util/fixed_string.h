#ifndef PRT3_FIXED_STRING_H
#define PRT3_FIXED_STRING_H

#include <algorithm>

namespace prt3 {

template<size_t N>
class FixedString {
public:
    enum {
        Size = N
    };

    FixedString() {}

    FixedString(char const * c) {
        char const * curr_c = c;
        char * curr_data = m_data;
        char * data_end = m_data + Size;
        while (curr_data < data_end &&
               *curr_c != '\0') {
            *curr_data = *curr_c;
            ++curr_c;
            ++curr_data;
        }
    }

    FixedString(FixedString const & other) {
       std::copy(other.m_data, other.m_data + Size, m_data);
    }

    FixedString & operator=(FixedString const & other) {
        std::copy(other.m_data, other.m_data + Size, m_data);
        return *this;
    }

    bool operator ==(FixedString const & other) const {
        char const * curr_this = m_data;
        char const * curr_other = other.m_data;
        char const * data_end = m_data + Size;
        while (curr_this < data_end) {
            if (*curr_this != *curr_other) { return false; }
            ++curr_this;
            ++curr_other;
        }
        return true;
    }

    bool operator !=(FixedString const & other) const {
        return !(*this == other);
    }

    bool operator ==(char const * c) const {
        char const * curr_c = c;
        char const * curr_data = m_data;
        char const * data_end = m_data + Size;
        while (curr_data < data_end &&
               *curr_c != '\0') {
            if (*curr_data != *curr_c) { return false; }
            ++curr_c;
            ++curr_data;
        }
        while (curr_data < data_end) {
            if (*curr_data != '\0') { return false; }
            ++curr_data;
        }
        return true;
    }

    bool operator !=(char const * c) const {
        return !(*this == c);
    }

    char * data() { return m_data; }
    char const * data() const { return m_data; }

    size_t size() const { return Size; }

private:
    char m_data[N] = { 0 };

    friend struct std::hash<prt3::FixedString<N> >;
};

} // namespace prt3

namespace std {
    template <size_t N> struct hash<prt3::FixedString<N> >   {
        size_t operator()(prt3::FixedString<N> const & s) const {
            size_t p = 16777619;
            size_t hash = 2166136261;

            char const * curr_data = s.m_data;
            char const * data_end = s.m_data + prt3::FixedString<N>::Size;
            while (curr_data != data_end) {
                hash = (hash ^ *curr_data) * p;
                ++curr_data;
            }

            hash += hash << 13;
            hash ^= hash >> 7;
            hash += hash << 3;
            hash ^= hash >> 17;
            hash += hash << 5;
            return hash;
        }
    };
}

#endif
