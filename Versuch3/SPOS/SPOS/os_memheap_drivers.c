#include "os_memheap_drivers.h"
#include "defines.h"
#include "os_memory_strategies.h"
#include <avr/pgmspace.h>

void os_initHeap (Heap *heap);

void os_initHeaps ();

Heap* os_lookupHeap (uint8_t index);

/*!
 *  Function needed by the Taskmanager, returns the number of Heaps.
 *
 *  \return The number of Heaps existing, e.g. 1 if there is only the intHeap
 */
size_t os_getHeapListLength (void);