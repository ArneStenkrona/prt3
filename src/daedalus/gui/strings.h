#ifndef DDS_STRINGS_H
#define DDS_STRINGS_H

#include <cstdint>

namespace dds {

typedef uint32_t StringIDType;

enum StringID : StringIDType {
    string_id_open,
    string_id_close,
    string_id_total_num
};

extern const char * const strings[string_id_total_num];

inline char const * id_to_string(StringIDType id) {
    return strings[id];
}

};

#endif // DDS_STRINGS_H
