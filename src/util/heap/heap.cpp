#include "heap.h"

#include <cassert>

using namespace prt3;

Heap::Heap(size_t capacity) {
    assert(m_heap_memory == nullptr);
    m_heap_memory = malloc(capacity);

    uintptr_t as_intptr = reinterpret_cast<uintptr_t>(m_heap_memory);
    ptrdiff_t align_diff = as_intptr % O1HEAP_ALIGNMENT;

    void * aligned_mem = reinterpret_cast<void*>(as_intptr + align_diff);
    size_t aligned_cap = capacity - align_diff;

    m_heap_instance = o1heapInit(aligned_mem, aligned_cap);
}

Heap::~Heap() {
    assert(m_heap_memory != nullptr);
    free(m_heap_memory);
    m_heap_memory = nullptr;
}

void * Heap::heap_malloc(size_t size) {
    void * mem = o1heapAllocate(m_heap_instance, size);
    m_size_map[mem] = size;
    return mem;
}

void Heap::heap_free(void * const pointer) {
    m_size_map.erase(pointer);
    o1heapFree(m_heap_instance, pointer);
}

void * Heap::heap_realloc(void * ptr, size_t size) {
    if (!ptr) return heap_malloc(size);
    void *newptr;
    size_t msize;
    msize = m_size_map.at(ptr);
    if (size <= msize)
        return ptr;
    newptr = heap_malloc(size);
    if (newptr != ptr) {
        memcpy(newptr, ptr, msize);
        heap_free(ptr);
    }
    return newptr;
}
