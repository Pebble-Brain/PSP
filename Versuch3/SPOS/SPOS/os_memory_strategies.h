/*! \file
 *  \brief Drivers for OS heap management.
 *
 *  Contains management of several logical heaps, each associated with one MemDriver.
 *
 *  \author Lehrstuhl Informatik 11 - RWTH Aachen
 */

#ifndef _OS_MEMORYSTRATEGIES_H
#define _OS_MEMORYSTRATEGIES_H

#include "os_memheap_drivers.h"

//! First-fit strategy
MemAddr os_Memory_FirstFit (Heap *heap, size_t size);

//! Next-fit strategy
MemAddr os_Memory_NextFit (Heap *heap, size_t size);

//! Worst-fit strategy
MemAddr os_Memory_WorstFit (Heap *heap, size_t size);

//! Best-fit strategy
MemAddr os_Memory_BestFit (Heap *heap, size_t size);

#endif