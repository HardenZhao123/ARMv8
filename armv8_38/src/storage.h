#ifndef STORAGE_H
#define STORAGE_H

#include <stdbool.h>
#include <stdint.h>

/**
 * @brief Size of memory required (in bytes)
 * 
 * 2 MB = 2 * 1024 * 1024
 */
#define MEMORY_SIZE (2 * 1024 * 1024)

/**
 * @brief Number of registers given (X0–X30)
 */
#define NUM_GENERAL_PURPOSE_REGISTERS 31

/**
 * @brief Condition flag register (PSTATE)
 * 
 * N — Negative flag  
 * Z — Zero flag  
 * C — Carry flag  
 * V — Overflow flag
 */
typedef struct {
    bool N, Z, C, V;
} StateRegister;

/**
 * @brief Register structure
 * 
 * It contains:
 * - 31 general-purpose registers (X0–X30)
 * - Zero register (ZR)
 * - Program counter (PC)
 * - Condition flags (PSTATE)
 */
typedef struct {
    uint64_t R[NUM_GENERAL_PURPOSE_REGISTERS]; // X0 to X30
    uint64_t ZR;                               // Zero Register
    uint64_t PC;                               // Program Counter
    StateRegister PSTATE;                      // Processor State Register
} Registers;

/**
 * @brief Complete processor state
 * 
 * It contains:
 * - Register set
 * - Memory array
 */
typedef struct {
    Registers registers;
    uint8_t memory[MEMORY_SIZE];
} Storage;

/**
 * @brief Creates and returns a fully zero-initialized Storage struct
 *
 * It includes:
 * - Zeroing out memory
 * - Initializing registers via `initialize_registers()`
 *
 * @return A Storage struct with cleared memory and default register state
 */
extern Storage empty_storage(void);

#endif
