#ifndef PRT3_UUID_H
#define PRT3_UUID_H

#include "src/util/random.h"

typedef uint64_t UUID;

inline UUID generate_uuid() { return random_int<UUID>(); }


#endif
