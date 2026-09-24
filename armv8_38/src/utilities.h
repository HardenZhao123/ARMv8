#ifndef UTILITIES_H
#define UTILITIES_H

#include "storage.h"


/**
* @brief Extracts an unsigned bitfield from a 32-bit instruction
 *
 * @param instr The 32-bit input instruction
 * @param msb Most significant bit index (inclusive)
 * @param lsb Least significant bit index (inclusive)
 * @return Extracted bitfield as an unsigned integer
 */
extern uint32_t extract_bits(uint32_t instr, int msb, int lsb);

/**
 * @brief Extracts a signed bitfield from a 32-bit instruction and sign-extends it
 *
 * @param instr The 32-bit input instruction
 * @param msb Most significant bit index (inclusive)
 * @param lsb Least significant bit index (inclusive)
 * @return Extracted and sign-extended value as a signed integer
 */
extern int32_t extract_bits_signed(uint32_t instr, int msb, int lsb);

/**
 * @brief Get the value of the 64-bit general register at reg_index
 *
 * Special handling for register index 31 (zero register)
 */
extern uint64_t get_x(Registers* regs, int reg_index);

/**
 * @brief // Set the value of the 64-bit general register at reg_index
 */
extern void set_x(Registers* regs, int reg_index, uint64_t value);

/**
 * @brief Get 32-bit value from the lower half of a register at reg_index
 */
extern uint32_t get_w(Registers* regs, int reg_index);

/**
 * @brief Set 32-bit value in the lower half of a register at reg_index
 */
extern void set_w(Registers* regs, int reg_index, uint32_t value);

/**
 * @brief Update PSTATE flags after a 64-bit arithmetic/logical operation
 */
extern void update_flags_64b(Storage* storage, uint64_t result, uint64_t a, uint64_t b, bool is_addition, bool is_arithmetic);

/**
 * @brief Update PSTATE flags after a 32-bit arithmetic/logical operation
 */
extern void update_flags_32b(Storage* storage, uint32_t result, uint32_t a, uint32_t b, bool is_addition, bool is_arithmetic);

/**
 * @brief Sign-extends a value with n bits to a 64-bit integer with the sign being preserved
 */
extern uint64_t sign_extend(uint64_t value, int n_bits);

/**
 * @brief Load a 64-bit value from memory at a given address
 *
 * returns 1 if successful, 0 otherwise
 */
extern int memory_load_64(const Storage* storage, uint64_t address, uint64_t *value);

/**
 * @brief Load a 32-bit value from memory at a given address
 *
 * returns 1 if successful, 0 otherwise
 */
extern int memory_load_32(const Storage* storage, uint64_t address, uint32_t *value);

/**
 * @brief Store a 64-bit value into memory at a given address
 */
extern void memory_store_64(Storage* storage, uint64_t address, uint64_t value);

/**
 * @brief Store a 32-bit value into memory at a given address
 */
extern void memory_store_32(Storage* storage, uint64_t address, uint32_t value);

/**
 * @brief Apply a shift operation to a 64-bit operand
 *
 * Supported shift types: 00 = LSL, 01 = LSR, 10 = ASR, 11 = ROR
 */
extern void apply_shift_64(uint8_t shift_type, uint8_t shift_amount, uint64_t *operand);

/**
 * @brief Apply a shift operation to a 32-bit operand
 *
 * Supported shift types:
 * 00 = LSL, 01 = LSR, 10 = ASR, 11 = ROR
 */
extern void apply_shift_32(uint8_t shift_type, uint8_t shift_amount, uint32_t *operand);

#endif
