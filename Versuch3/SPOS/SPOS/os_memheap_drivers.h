/*! \file
 *  \brief Drivers for OS heap management.
 *
 *  Contains management of several logical heaps, each associated with one MemDriver.
 *
 *  \author Lehrstuhl Informatik 11 - RWTH Aachen
 */

#ifndef _OS_MEMHEAP_DRIVERS_H
#define _OS_MEMHEAP_DRIVERS_H

#include "os_mem_drivers.h"
#include <stddef.h>


//! The structure of a heap driver which consists of a low level memory driver and heap specific information such as start, size etc...
typedef struct Heap {
    MemDriver driver;
    size_t startMap;
    size_t sizeMap;
    size_t sizePayload;
    AllocStrategy strategy;
    char const* name;
} Heap;

//! Macro to get pointer to the intHeap__ instance
#define intHeap (&intHeap__)

//! All available heap allocation strategies
typedef enum AllocStrategy {
    OS_MEM_FIRST,
    OS_MEM_NEXT,
    OS_MEM_BEST,
    OS_MEM_WORST,
} AllocStrategy;

//! This function clears the Heapmap of a specific heap.
void os_initHeap (Heap *heap);

//! Initialises all Heaps.
void os_initHeaps ();

//! Needed for Taskmanager interaction.
Heap* os_lookupHeap (uint8_t index);

//! Needed for Taskmanager interaction.
size_t 	os_getHeapListLength (void);

#endif