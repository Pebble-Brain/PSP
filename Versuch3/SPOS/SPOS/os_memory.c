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

static void setHighNibble(Heap *heap, MemAddr addr, MemValue value) {
    MemValue current = heap->driver->read(addr);
    MemValue updated = (current & 0x0F) | (value << 4);
    heap->driver->write(addr, updated);
}

static MemValue getLowNibble (const Heap *heap, MemAddr addr) {
    return (heap->driver->read(addr) & 0x0F);
}

static void setLowNibble(Heap *heap, MemAddr addr, MemValue value) {
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
    MemAddr offset   = addr - useStart;
    MemAddr mapAddr  = os_getMapStart(heap) + (offset / 2);

    switch (offset % 2) {
        case 0: // Fill the high nibble first
            setHighNibble(heap, mapAddr, value);
            break;
        case 1:
            setLowNibble(heap, mapAddr, value);
            break;
    }
}

/*! \brief This function is used to get a heap map entry on a specific heap
 *
 *  \param heap The heap from whos map the entry is supposed to be fetched
 *  \param addr	The address in use space for which the corresponding map entry shall be fetched
 */
MemValue os_getMapEntry	(const Heap* heap, MemAddr addr) {
    MemAddr useStart = os_getUseStart(heap);
    MemAddr offset   = addr - useStart;
    MemAddr mapAddr  = os_getMapStart(heap) + (offset / 2);

    switch (offset % 2) {
        case 0:
            getHighNibble(heap, mapAddr);
            break;
        case 1:
            getLowNibble(heap, mapAddr););
            break;
    }

}
//!  Get the size of a chunk on a given address.
uint16_t os_getChunkSize(const Heap *heap, MemAddr addr) {

}

//!  Get the address of the first byte of chunk.
MemAddr os_getFirstByteOfChunk(const Heap *heap, MemAddr addr) {

}

//!  Changes the memory management strategy.
void os_setAllocationStrategy(Heap *heap, AllocStrategy allocStrat) {
    heap->strategy = allocStrat;
}

//!  Returns the current memory management strategy.
AllocStrategy os_getAllocationStrategy(const Heap *heap) {
    return heap->strategy;
}
