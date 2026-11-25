#include "os_memory_strategies.h"
#include "os_memory.h"

MemAddr os_Memory_FirstFit (Heap *heap, size_t size) {
    MemAddr useStart = os_getUseStart(heap);
    MemAddr useEnd = useStart + os_getUseSize(heap);

    MemAddr candidate = useStart;
    size_t chunkSize = 0;

    while (candidate < useEnd && chunkSize < size) {
        if (os_getMapEntry(heap,candidate) == 0) {
            chunkSize++;
        } else {
            chunkSize = 0;
        }
        candidate++;
    }

    if (chunkSize < size) return 0; // If no free spaces were found and end has been reached, return 0
    return candidate - size;
}