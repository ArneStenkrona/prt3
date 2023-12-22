#ifndef PRT3_DATABASE_H
#define PRT3_DATABASE_H

#include "src/util/serialization_util.h"
#include "src/util/database/table.h"
#include "src/util/template_util.h"
#include "src/util/log.h"

#include <array>
#include <tuple>
#include <type_traits>
#include <vector>
#include <unordered_map>

namespace prt3 {

template<typename IDType, typename... DataTypes>
class Database {
public:
    template<template<typename...> class T, typename>
    struct wrap_taple_types { };

    template<template<typename...> class T, typename... Ts>
    struct wrap_taple_types<T, type_pack<Ts...> > {
        using type = T<Table<IDType, Ts>...>;
    };

    using TablesType = typename wrap_taple_types<std::tuple, type_pack<DataTypes...> >::type;

    enum {
        n_data_types = std::tuple_size_v<TablesType>
    };

    template<size_t I = 0>
    inline void register_pointers_and_functions() {
        auto & table = std::get<I>(m_tables);
        m_table_pointers[I] = reinterpret_cast<void*>(&table);
        m_add_funcs[I] =
            std::remove_reference<decltype(table)>::type::static_add;
        m_remove_funcs[I] =
            std::remove_reference<decltype(table)>::type::static_remove_entry;

        if constexpr(I+1 != n_data_types)
            register_pointers_and_functions<I+1>();
    }

    Database() {
        register_pointers_and_functions();
    }

    template<typename TableType, typename... ArgTypes>
    inline TableType & add_entry(
        IDType id,
        ArgTypes && ... args
    ) {
        return get_table<TableType>().add(id, args...);
    }

    inline void * add_entry_by_table_index(
        size_t table_index,
        IDType id,
        void * data
    ) {
        void * table_p = m_table_pointers[table_index];
        return m_add_funcs[table_index](table_p, id, data);
    }

    template<typename TableType>
    Table<IDType, TableType> const & get_table() const {
        return std::get<Index<Table<IDType, TableType>, TablesType>::value>(
            m_tables
        );
    }

    template<typename TableType>
    Table<IDType, TableType> & get_table() {
        return std::get<Index<Table<IDType, TableType>, TablesType>::value>(
            m_tables
        );
    }

    TablesType & get_tables() {
        return m_tables;
    }

    TablesType const & get_tables() const {
        return m_tables;
    }

    TablesType & get_table() {
        return m_tables;
    }

    /**
     * Note: reference should be considered stale if any data of same type is
     * added or removed
     */
    template<typename TableType>
    inline TableType & get_entry(IDType id) {
        return get_table<TableType>().get(id);
    }

    template<typename TableType>
    inline TableType const & get_entry(IDType id) const {
        return get_table<TableType>().get(id);
    }

    template<typename TableType>
    inline TableType * get_entries() {
        return get_table<TableType>().get_entries();
    }

    template<typename TableType>
    inline TableType const * get_entries() const {
        return get_table<TableType>().get_entries();
    }

    template<typename TableType>
    inline size_t num_entries() {
        return get_table<TableType>().num_entries();
    }

    template<typename TableType>
    inline bool has_entry(IDType id) const {
        return get_table<TableType>().has_entry(id);
    }

    template<typename TableType>
    inline bool remove_entry(IDType id) {
        return get_table<TableType>().remove_entry(id);
    }

    inline bool remove_entry_by_table_index(
        size_t table_index,
        IDType id
    ) {
        void * table_p = m_table_pointers[table_index];
        return m_remove_funcs[table_index](table_p, id);
    }

    inline void remove_all_components(IDType id)
    { remove_entries(id, m_tables); }

    inline void clear() {
        clear_tables();
    }

    void serialize(
        std::ostream & out,
        std::unordered_map<IDType, IDType> const & compacted_ids
    ) const {
        write_stream(out, std::tuple_size_v<TablesType>);
        serialize_table(out, compacted_ids);
    }

    void deserialize(std::istream & in) {
        size_t n_storages;
        read_stream(in, n_storages);
        deserialize_table(in, n_storages);
    }

    void serialize_for_id(
        std::ostream & out,
        IDType id
    ) const {
        write_stream(out, std::tuple_size_v<TablesType>);
        serialize_components(out, id);
    }

    void deserialize_for_id(
        std::istream & in,
        IDType id
    ) {
        size_t n_components;
        read_stream(in, n_components);
        deserialize_components(in, id, n_components);
    }

    template<typename TableType>
    inline void serialize_entry(
        std::ostream & out,
        IDType id
    ) const {
        get_table<TableType>().get(id).serialize(out);
    }

    template<typename TableType>
    inline void deserialize_entry(
        std::istream & in,
        IDType id
    ) {
        get_table<TableType>().add(id, in);
    }

    template<typename TableType>
    inline static constexpr size_t get_table_index()
    { return Index<Table<IDType, TableType>, TablesType>::value; }

private:
    TablesType m_tables;
    std::array<void*, n_data_types> m_table_pointers;

    std::array<
        decltype(&Table<IDType, typename std::tuple_element<0, TablesType>::type>::static_add),
        n_data_types
    > m_add_funcs;
    std::array<
        decltype(&Table<IDType, typename std::tuple_element<0, TablesType>::type>::static_remove_entry),
        n_data_types
    > m_remove_funcs;

    template<size_t I = 0, typename... Tp>
    inline void clear_tables() {
        auto & table = std::get<I>(m_tables);
        table.clear();

        if constexpr(I+1 != n_data_types)
            clear_tables<I+1>();
    }

    static constexpr uint64_t magic_num = 2407081819398577441ull;
    template<size_t I = 0>
    void serialize_table(
        std::ostream & out,
        std::unordered_map<IDType, IDType> const & compacted_ids
    ) const {
        uint64_t magic_num_i = magic_num + I;
        write_stream(out, magic_num_i);

        auto const & table = std::get<I>(m_tables);
        table.serialize(out, compacted_ids);

        if constexpr(I+1 != n_data_types)
            serialize_table<I+1>(out, compacted_ids);
    }

    template<size_t I = 0>
    void deserialize_table(
        std::istream & in,
        size_t n_tables
    ) {
        if (n_tables <= I) {
            return;
        }

        uint64_t magic_num_i = magic_num + I;
        uint64_t r_magic_num_i;
        read_stream(in, r_magic_num_i);
        if (r_magic_num_i != magic_num_i) {
            // TODO: error handling
            PRT3ERROR("deserialize_table(): error!\n");
        }

        auto & table = std::get<I>(m_tables);
        table.deserialize(in);

        if constexpr(I+1 != n_data_types)
            deserialize_table<I+1>(in, n_tables);
    }

    template<size_t I = 0>
    void serialize_entries(
        std::ostream & out,
        IDType id
    ) const {
        uint64_t magic_num_i = magic_num + I;
        write_stream(out, magic_num_i);

        auto const & table = std::get<I>(m_tables);
        if (table.has_entry(id)) {
            write_stream(out, (unsigned char)(1));
            table.get(id).serialize(out);
        } else {
            write_stream(out, (unsigned char)(0));
        }

        if constexpr(I+1 != n_data_types)
            serialize_entries<I+1>(out, id);
    }

    template<size_t I = 0>
    void deserialize_entries(
        std::istream & in,
        IDType id,
        size_t n_tables
    ) {
        if (n_tables <= I) {
            return;
        }

        uint64_t magic_num_i = magic_num + I;
        uint64_t r_magic_num_i;
        read_stream(in, r_magic_num_i);
        if (r_magic_num_i != magic_num_i) {
            // TODO: error handling
            PRT3ERROR("deserialize_entries(): error!\n");
        }

        unsigned char has;
        read_stream(in, has);

        if (has == 1) {
            auto & table = std::get<I>(m_tables);
            table.add(id, in);
        }

        if constexpr(I+1 != n_data_types)
            deserialize_entries<I+1>(in, id, n_tables);
    }

    template<size_t I = 0>
    inline void remove_entries(
        IDType id
    ) {
        auto & table = std::get<I>(m_tables);
        table.remove_entry(id);

        if constexpr(I+1 != n_data_types)
            remove_entries<I+1>(id);
    }
};

} // namespace prt3

#endif //  PRT3_DATABASE_H
