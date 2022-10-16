#ifndef PRT3_SIGNAL_H
#define PRT3_SIGNAL_H

#include <algorithm>

namespace prt3 {

class SignalString {
public:
    SignalString(char const * c) {
        char const * curr_c = c;
        char * curr_data = m_data;
        char * data_end = m_data + data_size;
        while (curr_data < data_end &&
               *curr_c != '\0') {
            *curr_data = *curr_c;
            ++curr_c;
            ++curr_data;
        }
    }

    SignalString(SignalString const & other) {
       std::copy(other.m_data, other.m_data + data_size, m_data);
    }

    SignalString & operator=(SignalString const & other) {
        std::copy(other.m_data, other.m_data + data_size, m_data);
        return *this;
    }

    bool operator ==(SignalString const & other) const {
        char const * curr_this = m_data;
        char const * curr_other = other.m_data;
        char const * data_end = m_data + data_size;
        while (curr_this < data_end) {
            if (*curr_this != *curr_other) { return false; }
            ++curr_this;
            ++curr_other;
        }
        return true;
    }

    bool operator !=(SignalString const & other) const {
        return !(*this == other);
    }

    bool operator ==(char const * c) const {
        char const * curr_c = c;
        char const * curr_data = m_data;
        char const * data_end = m_data + data_size;
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

private:
    static constexpr size_t data_size = 32;
    char m_data[data_size] = { 0 };

    friend struct std::hash<prt3::SignalString>;
};

} // namespace prt3

namespace std {
    template <> struct hash<prt3::SignalString>   {
        size_t operator()(prt3::SignalString const & s) const {
            size_t p = 16777619;
            size_t hash = 2166136261;

            char const * curr_data = s.m_data;
            char const * data_end = s.m_data + prt3::SignalString::data_size;
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
