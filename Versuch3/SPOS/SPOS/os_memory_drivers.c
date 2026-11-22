#include "os_mem_drivers.h"
#include "defines.h"

void initSRAM_internal(void);

/*! \brief Reads a single byte from the specified SRAM address.
 *
 *  \param addr 16-bit memory address to read from
 *  \return The byte value stored at the specified address
 */
MemValue readSRAM_internal (MemAddr addr) {
    return *(uint8_t*)addr; // Casts 16 bit MemAddr into a pointer that points to an 8 bit memory atom
}

/*! \brief Writes a single byte to the specified SRAM address.
 *
 *  \param addr 16-bit memory address to write to
 *  \param value The byte value to write
 */
void writeSRAM_internal (MemAddr addr, MemValue value) {
    *(uint8_t*)addr = value;
}

/*! \brief Memory driver instance for the internal SRAM.
 *  Note the initialisation is not strictly necessary.
 */
MemDriver intSRAM__ = {
    initSRAM_internal,
    readSRAM_internal,
    writeSRAM_internal
};

/*! \brief Initializes all memory devices.
 *
 *  This function calls the init function of each registered memory driver.
 */
void initMemoryDevices() {
    intSRAM->init;
}