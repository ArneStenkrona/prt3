#ifndef PRT3_FIXED_STRING_H
#define PRT3_FIXED_STRING_H

#include <algorithm>

namespace prt3 {

template<size_t N>
class FixedString {
    static_assert(N > 0);
public:
    enum {
        Size = N
    };

    FixedString() {}

    FixedString(char const * c) {
        char const * curr_c = c;
        char * curr_data = m_data;
        char * data_end = m_data + (Size - 1);
        while (curr_data < data_end &&
               *curr_c != '\0') {
            *curr_data = *curr_c;
            ++curr_c;
            ++curr_data;
        }
    }

    FixedString(FixedString const & other) {
        memcpy(m_data, other.m_data, Size - 1);
    }

    FixedString(FixedString && other) {
        memcpy(m_data, other.m_data, Size - 1);
    }

    FixedString & operator=(FixedString const & other) {
        char const * curr_o = other.m_data;
        char * curr_data = m_data;
        char * data_end = m_data + (Size - 1);
        while (curr_data < data_end &&
               *curr_o != '\0') {
            *curr_data = *curr_o;
            ++curr_o;
            ++curr_data;
        }
        *curr_data = '\0';
        return *this;
    }

    FixedString& operator=(FixedString && other) {
        char const * curr_o = other.m_data;
        char * curr_data = m_data;
        char * data_end = m_data + (Size - 1);
        while (curr_data < data_end &&
               *curr_o != '\0') {
            *curr_data = *curr_o;
            ++curr_o;
            ++curr_data;
        }
        *curr_data = '\0';
        return *this;
    }

    FixedString & operator=(char const * c) {
        char const * curr_c = c;
        char * curr_data = m_data;
        char * data_end = m_data + (Size - 1);
        while (curr_data < data_end &&
               *curr_c != '\0') {
            *curr_data = *curr_c;
            ++curr_c;
            ++curr_data;
        }
        *curr_data = '\0';
        return *this;
    }

    bool operator ==(FixedString const & other) const {
        char const * curr_this = m_data;
        char const * curr_other = other.m_data;
        char const * data_end = m_data + Size;
        while (curr_this < data_end) {
            if (*curr_this != *curr_other) { return false; }
            if (*curr_this == '\0') return true;
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
            if (*curr_data == '\0') return true;
            ++curr_c;
            ++curr_data;
        }
        return true;
    }

    bool operator !=(char const * c) const {
        return !(*this == c);
    }

    char * data() { return m_data; }
    char const * data() const { return m_data; }

    size_t buf_size() const { return Size; }
    size_t writeable_size() const { return Size - 1; }

    size_t len() const {
        char const * curr = data();
        while (*curr != '\0') {
            ++curr;
        }
        return static_cast<size_t>(curr - data());
    }

    void clear() {
        m_data[0] = '\0';
    }

private:
    char m_data[N] = { 0 };

    friend struct std::hash<prt3::FixedString<N> >;
};

} // namespace prt3

namespace std {
    template <size_t N> struct hash<prt3::FixedString<N> > {
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
