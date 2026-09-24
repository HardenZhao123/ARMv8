#include <string.h>

#include "storage.h"

/**
 * @brief Initialize all registers
 *
 * Registers are zeroed and PSTATE is initialized with:
 * - Z (Zero) = true
 * - N (Negative), C (Carry), V (Overflow) = false
 *
 * @param registers Pointer to the register set to initialize
 */
static void initialize_registers(Registers *registers) {
    memset(registers, 0, sizeof(Registers));
    registers->PSTATE.Z = true;
    registers->PSTATE.N = false;
    registers->PSTATE.C = false;
    registers->PSTATE.V = false;
}

Storage empty_storage(void) {
    Storage storage;
    memset(storage.memory, 0, sizeof(storage.memory));
    initialize_registers(&storage.registers);
    return storage;
}
