#ifndef PRT3_TABLE_H
#define PRT3_TABLE_H

#include "src/util/log.h"

#include <vector>
#include <unordered_map>

namespace prt3 {

template<typename IDType, typename... DataTypes>
class Database;

template<typename IDType, typename T>
class Table {
public:
    using data_type = T;

    template<typename... ArgTypes>
    inline T & add(IDType id, ArgTypes && ... args) {
        if (static_cast<IDType>(m_id_map.size()) <= id) {
            m_id_map.resize(id + 1, NO_ENTRY);
        }

        if (!has_entry(id)) {
            m_id_map[id] = m_entries.size();
            m_index_map.emplace_back(id);
            m_entries.emplace_back(args...);
        }

        return get(id);
    }

    inline static void * static_add(
        void * table_p,
        IDType id,
        void * entry_p
    ) {
        Table<IDType, T> & t = *reinterpret_cast<Table<IDType, T> *>(table_p);
        T & entry = t.add(id, *reinterpret_cast<T*>(entry_p));
        return reinterpret_cast<void*>(&entry);
    }

    inline T const & get(IDType id) const {
        return m_entries[m_id_map[id]];
    }

    inline T & get(IDType id) {
        return m_entries[m_id_map[id]];
    }

    inline bool has_entry(IDType id) const {
        return static_cast<IDType>(m_id_map.size()) > id &&
               m_id_map[id] != NO_ENTRY;
    }

    inline T * get_entries()
    { return m_entries.data(); }

    inline T const * get_entries() const
    { return m_entries.data(); }

    inline IDType const * index_map() const {
        return m_index_map.data();
    }

    inline size_t num_entries() const
    { return m_entries.size(); }

    inline void serialize(
        std::ostream & out,
        std::unordered_map<IDType, IDType> const & compacted_ids
    ) const {
        write_stream(out, m_entries.size());
        uint32_t index = 0;
        for (T const & entry : m_entries) {
            write_stream(out, compacted_ids.at(m_index_map[index]));
            entry.serialize(out);
            ++index;
        }
    }

    inline void deserialize(
        std::istream & in
    ) {
        size_t n_entries;
        read_stream(in, n_entries);
        for (size_t i = 0; i < n_entries; ++i) {
            IDType id;
            read_stream(in, id);
            add(id, in);
        }
    }

    inline void clear() {
        m_id_map.clear();
        m_index_map.clear();
        m_entries.clear();
    }

    bool remove_entry(IDType id) {
        if (!has_entry(id)) {
            return false;
        }

        IDType index = m_id_map[id];
        m_id_map[id] = NO_ENTRY;

        if (index + 1 != m_entries.size()) {
            IDType swap = m_index_map.back();
            m_entries[index] = std::move(m_entries.back());
            m_id_map[swap] = index;
            m_index_map[index] = swap;
        }

        m_index_map.pop_back();
        m_entries.pop_back();

        return true;
    }

    inline static bool static_remove_entry(void * table_p, IDType id) {
        Table<IDType, T> & t = *reinterpret_cast<Table<IDType, T> *>(table_p);
        return t.remove_entry(id);
    }

    inline IDType index_to_id(size_t index) const {
        return m_index_map[index];
    }

private:
    static constexpr IDType NO_ENTRY = -1;
    std::vector<uint32_t> m_id_map;
    std::vector<IDType> m_index_map;
    std::vector<T> m_entries;
};

} // namespace prt3

#endif // PRT3_TABLE_H
