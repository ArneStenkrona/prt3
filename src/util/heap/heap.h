#ifndef PRT3_HEAP_H
#define PRT3_HEAP_H

#include "o1heap.h"

#include <unordered_map>

#include <cstring>

namespace prt3 {

class Heap {
public:
    Heap(size_t capacity);
    ~Heap();

    void * heap_malloc(size_t size);
    void heap_free(void * const pointer);
    void * heap_realloc(void *ptr, size_t size);

private:
    void * m_heap_memory = nullptr;
    O1HeapInstance * m_heap_instance = nullptr;;

    std::unordered_map<void *, size_t> m_size_map;
};

} // namespace prt3

#endif // PRT3_HEAP_H
