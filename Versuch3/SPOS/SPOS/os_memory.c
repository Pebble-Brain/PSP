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
 *  \returns The chunk's length.
 */
uint16_t os_getChunkSize(const Heap *heap, MemAddr addr) {
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

/*! \brief Allocates a chunk of memory on the medium given by the driver and reserves it for the current process.
 *  \param heap	The heap to be used
 *  \param size The amount of memory to be allocated in Bytes. Must be able to handle a single byte and values greater than 255.
 *  \returns A pointer to the first Byte of the allocated chunk. 0 if allocation fails (0 is never a valid address).
*/
MemAddr os_malloc (Heap* heap, size_t size) { 
    os_enterCriticalSection();
    ProcessID pid = os_getCurrentProc();
    MemAddr start = 0;

    switch (os_getAllocationStrategy(heap)) {
        case OS_MEM_FIRST:
            start = os_Memory_FirstFit(heap, size);
            break;
        default:
            break;
    }

    // Catch failure of allocation
    if (start == 0) {
        os_leaveCriticalSection();
        return 0;
    }

    setMapEntry(heap, start, pid); // Mark start of chunk with PID
    for (MemAddr index = start + 1; index < start + size; index++) {
        setMapEntry(heap, index, 0xF); // Set continuation bytes
    }

    os_leaveCriticalSection();
    return start;
}

/*! \brief Frees a chunk of allocated memory on the medium given by the driver. 
 * This function checks if the call has been made by someone with the right to do it 
 * (i.e. the process that owns the memory or the OS). This function is made 
 * in order to avoid code duplication and is called by several functions that, 
 * in some way, free allocated memory such as os_freeProcessMemory/os_free...
 *  \param heap	The heap to be used
 *  \param addr An address inside of the chunk (not necessarily the start).
 *  \param owner The expected owner of the chunk to be freed
*/
void os_freeOwnerRestricted (Heap* heap, MemAddr addr, ProcessID owner) {
    os_enterCriticalSection();
    MemAddr start = os_getFirstByteOfChunk(heap, addr);
    size_t size = os_getChunkSize(heap, start);

    // Verify that the chunk belongs to the process
    if(os_getMapEntry(heap, start) != owner) {
        os_leaveCriticalSection();
        return;
    }

    for (MemAddr index = start; index < start + size; index++) {
        setMapEntry(heap, index, 0); // Clear map bytes
    }

    os_leaveCriticalSection();
}

/*! \brief Frees a chunk of allocated memory of the currently running process on the given heap
 *  \param heap	The heap to be used
 *  \param addr An address inside of the chunk (not necessarily the start).
*/
void os_free(Heap* heap, MemAddr addr) {
    ProcessID current = os_getCurrentProc();
    os_freeOwnerRestricted(heap, addr, current);
}

/*! \brief This function realises the garbage collection. When called, every allocated memory chunk of the given process is freed
 *  \param heap	The heap on which we look for allocated memory
 *  \param pid The ProcessID of the process that owns all the memory to be freed
*/
void os_freeProcessMemory (Heap* heap, ProcessID pid) {
    
    // Process 0 (idle) cannot own dynamic memory; 0x0 in the Map means free
    if (pid == 0 || pid >= MAX_NUMBER_OF_PROCESSES) return;

    os_enterCriticalSection();
    MemAddr start = os_getUseStart(heap);
    size_t size = os_getUseSize(heap);
    MemAddr end = start + size;

    for(MemAddr candidate = start; candidate < end; candidate++) {
        if (os_getMapEntry(heap, candidate) == pid) { // Find matching PIDs in the map area
            size_t chunkSize = os_getChunkSize(heap, candidate); 
            os_freeOwnerRestricted(heap, candidate, pid); // Clear chunk
            candidate += chunkSize-1; // Advance the search by the chunk size (note outside loop increments by 1)
        }
    }
    
    os_leaveCriticalSection();
}