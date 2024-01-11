#ifndef PRT3_AUDIO_MEM_UTIL_H
#define PRT3_AUDIO_MEM_UTIL_H

#include <cstring>

namespace prt3 {

void init_o1h_instance(size_t capacity);
void free_o1h_instance();

void * o1hmalloc(size_t size);

void o1hfree(void * const pointer);

void * o1hrealloc(void *ptr, size_t size);

} // namespace prt3

#endif // PRT3_AUDIO_MEM_UTIL_H
