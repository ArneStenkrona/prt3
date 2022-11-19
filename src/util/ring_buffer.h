#ifndef PRT3_RING_BUFFER_H
#define PRT3_RING_BUFFER_H

#include <array>
#include <cstdlib>

namespace prt3 {

template<typename T, size_t N>
class RingBuffer {
    static_assert(N > 0);
public:
    enum {
        Size = N
    };

    void push_back(T const & val) {
        m_buffer[m_head] = val;
        ++m_head;
        m_head %= Size;

        if (m_head == m_tail) {
            m_tail = m_head + 1;
            m_tail %= Size;
        }
    }

    void pop_back() {
        if (empty()) {
            std::abort();
        }
        m_head = (m_head + Size - 1) % Size;
    }

    T const & back() const {
        if (empty()) {
            std::abort();
        }
        return m_buffer[(m_head + Size - 1) % Size];
    }

    T & back() {
        if (empty()) {
            std::abort();
        }
        return m_buffer[(m_head + Size - 1) % Size];
    }

    T const & front() const {
        if (empty()) {
            std::abort();
        }
        return m_buffer[m_tail];
    }

    T & front() {
        if (empty()) {
            std::abort();
        }
        return m_buffer[m_tail];
    }

    bool empty() {
        return m_tail == m_head;
    }

    bool full() {
        return size() == Size;
    }

    size_t size() {
        if (m_head >= m_tail) {
            return m_head - m_tail;
        } else {
            return Size - (m_tail - m_head);
        }
    }

private:
    std::array<T, N> m_buffer;
    size_t m_tail = 0;
    size_t m_head = 0;
};

} // namespace prt3

#endif
