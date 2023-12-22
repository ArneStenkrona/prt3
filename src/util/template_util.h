#ifndef PRT3_TEMPLATE_UTIL_H
#define PRT3_TEMPLATE_UTIL_H

#include <cstddef>
#include <tuple>

namespace prt3 {

template<typename... Args>
struct type_pack { };

template <class T, class Tuple>
    struct Index;

template <class T, class... Types>
struct Index<T, std::tuple<T, Types...> > {
    static const std::size_t value = 0;
};

template <class T, class U, class... Types>
struct Index<T, std::tuple<U, Types...> > {
    static const std::size_t value = 1 + Index<T, std::tuple<Types...> >::value;
};

} // namespace prt3

#endif
