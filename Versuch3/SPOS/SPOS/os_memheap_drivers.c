#include "os_memheap_drivers.h"
#include "defines.h"
#include "os_memory_strategies.h"
#include <avr/pgmspace.h>

Heap intHeap__ = {
    .driver = intSRAM,
    .startMap = HEAPOFFSET + AVR_SRAM_START,
    .sizeMap = HEAP_MAP_SIZE,
    .sizePayload = HEAP_PAYLOAD_SIZE,
    .strategy = OS_MEM_FIRST,
    .name = "mainHeap"
};

void os_initHeap (Heap *heap);

void os_initHeaps ();

Heap* os_lookupHeap (uint8_t index);

/*!
 *  Function needed by the Taskmanager, returns the number of Heaps.
 *
 *  \return The number of Heaps existing, e.g. 1 if there is only the intHeap
 */
size_t os_getHeapListLength (void);