#include "audio_mem_util.h"

#include "src/util/log.h"

#include "o1heap.h"

#include <unordered_map>
#include <cassert>

using namespace prt3;

void * o1_heap_memory = nullptr;
O1HeapInstance * o1_heap_instance = nullptr;;

std::unordered_map<void *, size_t> size_map;

void prt3::init_o1h_instance(size_t capacity) {
    assert(o1_heap_memory == nullptr);
    o1_heap_memory = malloc(capacity);

    uintptr_t as_intptr = reinterpret_cast<uintptr_t>(o1_heap_memory);
    ptrdiff_t align_diff = as_intptr % O1HEAP_ALIGNMENT;

    void * aligned_mem = reinterpret_cast<void*>(as_intptr + align_diff);
    size_t aligned_cap = capacity - align_diff;

    o1_heap_instance = o1heapInit(aligned_mem, aligned_cap);
}

void prt3::free_o1h_instance() {
    assert(o1_heap_memory != nullptr);
    free(o1_heap_memory);
    o1_heap_memory = nullptr;
}

void * prt3::o1hmalloc(size_t size) {
    void * mem = o1heapAllocate(o1_heap_instance, size);
    size_map[mem] = size;
    return mem;
}

void prt3::o1hfree(void * const pointer) {
    size_map.erase(pointer);
    o1heapFree(o1_heap_instance, pointer);
}

void * prt3::o1hrealloc(void * ptr, size_t size) {
    if (!ptr) return o1hmalloc(size);
    void *newptr;
    size_t msize;
    msize = size_map.at(ptr);
    if (size <= msize)
        return ptr;
    newptr = o1hmalloc(size);
    if (newptr != ptr) {
        memcpy(newptr, ptr, msize);
        o1hfree(ptr);
    }
    return newptr;
}
