#ifndef PIPELINE_H
#define PIPELINE_H

#include "instructions.h"
#include "storage.h"

/**
 * @brief Executes a single decoded instruction
 * 
 * This dispatches execution based on instruction type and updates the processor state
 * 
 * @param instr Pointer to a decoded instruction
 * @param storage Pointer to the Storage
 */
extern void execute_instruction(const Instruction *instr, Storage *storage);

/**
 * @brief Main emulation loop
 * 
 * It includes:
 * - Fetches, decodes, and executes instructions sequentially
 * - Halt when halt instruction defined is encountered
 * 
 * @param storage Pointer to initialized processor state
 * @return Total number of executed instructions
 */
extern int emulate(Storage *storage);

#endif
