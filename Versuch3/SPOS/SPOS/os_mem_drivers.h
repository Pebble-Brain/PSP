/*! \file
 *  \brief Drivers for OS memory management. 
 *
 *  Contains management of several RAM devices of the OS. It contains everything that is associated with low level memory access.
 *
 *  \author Lehrstuhl Informatik 11 - RWTH Aachen
 */

#ifndef _OS_MEM_DRIVERS_H
#define _OS_MEM_DRIVERS_H

#include <stdint.h>

//! Realises a pointer to the MemDriver intSRAM__.
#define intSRAM (&intSRAM__)

//! Type used instead of uint8_t* pointers to avoid direct dereferencing.
typedef uint16_t MemAddr; 

//! Type for a single value (used instead of uint8_t to increase readability)
typedef uint8_t MemValue; 

//! Type of a memory driver initialisation function.
typedef void (*MemoryInitHnd)(void);

//! Type of a memory driver read function.
typedef MemValue (*MemoryReadHnd)(MemAddr addr);

//! Type of a memory driver write function.
typedef void (*MemoryWriteHnd)(MemAddr addr, MemValue value);

//! The data structure for a memory driver such as intSRAM.
typedef struct MemDriver {
    MemoryInitHnd init;
    MemoryReadHnd read;
    MemoryWriteHnd write;
} MemDriver;

//! Initialise all memory devices.
void initMemoryDevices();

#endif