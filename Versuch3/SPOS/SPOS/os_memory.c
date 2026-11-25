#include "os_memory.h"
#include "os_core.h"
#include "os_memory_strategies.h"
#include "util.h"


//! Get the size of the heap-map.
size_t os_getMapSize(const Heap *heap) {
    return heap->sizeMap;
}

//!  Get the size of the usable heap.
size_t os_getUseSize(const Heap *heap) {
    return heap->sizePayload;
}

//!  Get the start of the heap-map.
MemAddr os_getMapStart(const Heap *heap) {
    return heap->startMap;
}

//!  Get the start of the usable heap.
MemAddr os_getUseStart(const Heap *heap) {
    return heap->startMap + heap->sizeMap;
}

static MemValue getHighNibble (const Heap *heap, MemAddr addr) {
    return (heap->driver->read(addr) >> 4);
}

static void setHighNibble(const Heap *heap, MemAddr addr, MemValue value) {
    MemValue current = heap->driver->read(addr);
    MemValue updated = (current & 0x0F) | (value << 4);
    heap->driver->write(addr, updated);
}

static MemValue getLowNibble (const Heap *heap, MemAddr addr) {
    return (heap->driver->read(addr) & 0x0F);
}

static void setLowNibble(const Heap *heap, MemAddr addr, MemValue value) {
    MemValue current = heap->driver->read(addr);
    MemValue updated = (current & 0xF0) | (0x0F & value);
    heap->driver->write(addr, updated);
}

/*! \brief Assigns a value in the map corresponding a memory address 
 *
 *  \param heap The heap on whos map the entry is supposed to be set
 *  \param addr	The address in use space for which the corresponding map entry shall be set
 *  \param value The value that is supposed to be set onto the map (valid range: 0x0 - 0xF)
 */
static void setMapEntry(const Heap *heap, MemAddr addr, MemValue value) {
    MemAddr useStart = os_getUseStart(heap);
    MemAddr offset = addr - useStart;
    MemAddr mapAddr = os_getMapStart(heap) + (offset / 2);

    if (offset % 2) {
        setLowNibble(heap, mapAddr, value);
    } else {
        setHighNibble(heap, mapAddr, value);
    }
}

/*! \brief This function is used to get a heap map entry on a specific heap
 *
 *  \param heap The heap from whos map the entry is supposed to be fetched
 *  \param addr	The address in use space for which the corresponding map entry shall be fetched
 */
MemValue os_getMapEntry	(const Heap* heap, MemAddr addr) {
    MemAddr useStart = os_getUseStart(heap);
    MemAddr offset = addr - useStart;
    MemAddr mapAddr = os_getMapStart(heap) + (offset / 2);

    if (offset % 2) {
        return getLowNibble(heap, mapAddr);
    } else {
        return getHighNibble(heap, mapAddr);
    }      
}

/*! \brief This function is used to determine where a chunk starts if a given address might not point to the start of the chunk but to some place inside of it.
 *
 *  \param heap	The heap the chunk is on hand in
 *  \param addr	The address that points to some byte of the chunk
 */
MemAddr os_getFirstByteOfChunk(const Heap *heap, MemAddr addr) {
    MemAddr useStart = os_getUseStart(heap);
    MemAddr useEnd = useStart + os_getUseSize(heap);

    // Check that the payload address is valid and does point to a chunk
    if (addr < useStart || addr >= useEnd) return 0;
    if (os_getMapEntry(heap, addr) == 0x0) return 0;

    // Decrement the address until is no longer a continuation, or reaches the start of the heap
    MemAddr candidate = addr;
    while (candidate > useStart && os_getMapEntry(heap, candidate) == 0xF) candidate--;
    return candidate;
}

/*! \brief Takes a use-pointer and computes the length of the chunk. This only works for occupied chunks. The size of free chunks is always 0.
 *
 *  \param heap	The heap to be used
 *  \param addr	The address that points to some byte of the chunk
 */uint16_t os_getChunkSize(const Heap *heap, MemAddr addr) {
    MemAddr useStart = os_getUseStart(heap);
    MemAddr useEnd = useStart + os_getUseSize(heap);

    // Check that the payload address is valid and does point to a chunk
    MemAddr start = os_getFirstByteOfChunk(heap, addr);
    if (start == 0) return 0;

    // Incrementfd the address until is no longer a continuation, or reaches the start of the heap
    MemAddr end = addr + 1;
    while (end < useEnd && os_getMapEntry(heap, end) == 0xF) end++;
    return end - start;
}

//!  Changes the memory management strategy.
void os_setAllocationStrategy(Heap *heap, AllocStrategy allocStrat) {
    heap->strategy = allocStrat;
}

//!  Returns the current memory management strategy.
AllocStrategy os_getAllocationStrategy(const Heap *heap) {
    return heap->strategy;
}
