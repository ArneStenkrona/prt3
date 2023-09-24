#ifndef PRT3_LRU_CACHE_H
#define PRT3_LRU_CACHE_H

#include <cstddef>
#include <queue>
#include <unordered_map>



namespace prt3 {

template<typename Key, typename T, size_t N>
class LRUCache {
public:
    static_assert(N > 1);

    LRUCache() {
        for (size_t i = 0; i < N; ++i) {
            m_info[i].prev = (N + i - 1) % N;
            m_info[i].next = (N + i + 1) % N;
        }
    }

    bool has_key(Key const & key) const {
        return m_key_to_entry.find(key) != m_key_to_entry.end();
    }

    T * access(Key const & key) {
        auto it = m_key_to_entry.find(key);
        if (it != m_key_to_entry.end()) {
            set_most_recent(it->second);
            return &m_entries[it->second];
        }
        return nullptr;
    }

    T * push_new_entry(Key const & key) {
        size_t index = m_info_head;
        auto it_key = m_entry_to_key.find(index);
        if (it_key != m_entry_to_key.end()) {
            m_key_to_entry.erase(it_key->second);
            m_entry_to_key.erase(index);
        }

        m_key_to_entry[key] = index;
        m_entry_to_key[index] = key;
        set_most_recent(index);
        return &m_entries[index];
    }

    void invalidate(Key const & key) {
        size_t index = m_key_to_entry.at(key);
        set_least_recent(index);
        m_key_to_entry.erase(key);
        m_entry_to_key.erase(index);
    }

private:
    T m_entries[N];
    std::unordered_map<Key, size_t> m_key_to_entry;
    std::unordered_map<size_t, Key> m_entry_to_key;

    struct LRUInfo {
        size_t prev;
        size_t next;
    };

    LRUInfo m_info[N];
    size_t m_info_head = 0;
    size_t m_info_tail = N - 1;

    void set_most_recent(size_t index) {
        if (index == m_info_tail) return;
        size_t curr_head = m_info_head;
        if (index == m_info_head) {
            m_info_head = m_info[index].next;
        }

        m_info[m_info[index].next].prev = m_info[index].prev;
        m_info[m_info[index].prev].next = m_info[index].next;
        m_info[index].next = curr_head;
        m_info[index].prev = m_info_tail;
        m_info[m_info_tail].next = index;
        m_info[m_info_head].prev = index;
        m_info_tail = index;
    }

    void set_least_recent(size_t index) {
        if (index == m_info_head) return;
        size_t curr_tail = m_info_tail;
        if (index == m_info_tail) {
            m_info_tail = m_info[index].prev;
        }

        m_info[m_info[index].next].prev = m_info[index].prev;
        m_info[m_info[index].prev].next = m_info[index].next;
        m_info[index].next = m_info_head;
        m_info[index].prev = curr_tail;
        m_info[m_info_tail].next = index;
        m_info[m_info_head].prev = index;
        m_info_head = index;
    }
};

} // namespace prt3

#endif // PRT3_LRU_CACHE_H
