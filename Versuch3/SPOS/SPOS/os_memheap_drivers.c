#include "os_memheap_drivers.h"
#include "defines.h"
#include "os_memory_strategies.h"
#include <avr/pgmspace.h>

/*! \brief Memory driver instance for the internal heap.
 *
 *  Heap instance for internal SRAM.
 *  The heap starts after the global variables (HEAPOFFSET) and extends
 *  up to the process stack area.
 */
Heap intHeap__ = {
    .driver = &intSRAM__,
    .startMap = HEAPOFFSET + AVR_SRAM_START,
    .sizeMap = HEAP_MAP_SIZE,
    .sizePayload = HEAP_PAYLOAD_SIZE,
    .strategy = OS_MEM_FIRST,
    .name = "mainHeap"
};

//! Number of heaps in the system
static uint8_t numHeaps = NUM_HEAPS;

//! Array of heap pointers for lookup by index
static Heap* heaps[NUM_HEAPS] = {&intHeap__};

/*! \brief Clears the heap map of a specific heap.
 *
 *  This function writes 0 to all bytes in the map area of the given heap,
 *  marking all blocks as free.
 *
 *  \param heap Pointer to the heap to initialize
 */
void os_initHeap (Heap *heap) {
    for (size_t i = 0; i < heap->sizeMap; i++) {
        heap->driver->write(heap->startMap + i, 0);
    }
}

/*! \brief Initializes all registered heaps.
 */
void os_initHeaps () {
    for (uint8_t i = 0; i < numHeaps; i++) {
        os_initHeap(heaps[i]);
    }
}

/*! \brief Looks up a heap by its index.
 *
 *  \param index The index of the heap to retrieve (0 for SRAM)
 *  \return Pointer to the heap
 */
Heap* os_lookupHeap (uint8_t index) {
    return heaps[index];
}

/*! \brief Returns the number of registered heaps.
 *
 *  \return The number of heaps in the system
 */
uint8_t os_getHeapListLength (void) {
    return numHeaps;
}