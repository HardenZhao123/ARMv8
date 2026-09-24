#include <assert.h>
#include <inttypes.h>
#include <string.h>
#include <stdio.h>

#include "utilities.h"

uint32_t extract_bits(uint32_t instr, int msb, int lsb) {
    uint32_t mask = (1U << (msb - lsb + 1)) - 1;
    return (instr >> lsb) & mask;
}

int32_t extract_bits_signed(uint32_t instr, int msb, int lsb) {
    uint32_t mask = (1U << (msb - lsb + 1)) - 1;
    return (int32_t)sign_extend((instr >> lsb) & mask, msb - lsb + 1);
}

uint64_t get_x(Registers* regs, int reg_index) {
    if (reg_index == 31) {
        regs -> ZR = (uint64_t)0;
        return regs -> ZR;
    }
    assert(0 <= reg_index && reg_index < NUM_GENERAL_PURPOSE_REGISTERS);
    return regs -> R[reg_index];
}

void set_x(Registers* regs, int reg_index, uint64_t value) {
    if (reg_index == 31) {
        regs -> ZR = (uint64_t)0;
        return;
    }
    assert(0 <= reg_index && reg_index < NUM_GENERAL_PURPOSE_REGISTERS);
    regs -> R[reg_index] = value;
}

uint32_t get_w(Registers* regs, int reg_index) {
    if (reg_index == 31) {
        regs -> ZR = (uint64_t)0;
        return (uint32_t)(regs -> ZR);
    }
    assert(0 <= reg_index && reg_index < NUM_GENERAL_PURPOSE_REGISTERS);
    return (uint32_t)(regs -> R[reg_index] & 0xFFFFFFFF);
}

void set_w(Registers* regs, int reg_index, uint32_t value) {
    if (reg_index == 31) {
        regs -> ZR = (uint64_t)0;
        return;
    }
    assert(0 <= reg_index && reg_index < NUM_GENERAL_PURPOSE_REGISTERS);
    regs -> R[reg_index] = (uint64_t)value;
}

void update_flags_64b(Storage* storage, uint64_t result, uint64_t a, uint64_t b, bool is_addition, bool is_arithmetic) {
    storage->registers.PSTATE.N = (result >> 63) == 1;
    storage->registers.PSTATE.Z = (result == 0);
    if (is_arithmetic) {
        if (is_addition) {
            storage->registers.PSTATE.C = result < a;
            storage->registers.PSTATE.V = ((int64_t)a > 0 && (int64_t)b > 0 && (int64_t)result < 0) ||
                        ((int64_t)a < 0 && (int64_t)b < 0 && (int64_t)result > 0);
        } else {
            storage->registers.PSTATE.C = a >= b;
            storage->registers.PSTATE.V = ((int64_t)a < 0 && (int64_t)b > 0 && (int64_t)result > 0) ||
                        ((int64_t)a > 0 && (int64_t)b < 0 && (int64_t)result < 0);
        }
    } else {
        storage->registers.PSTATE.C = false;
        storage->registers.PSTATE.V = false;
    }
}

void update_flags_32b(Storage* storage, uint32_t result, uint32_t a, uint32_t b, bool is_addition, bool is_arithmetic)
{
    storage->registers.PSTATE.N = (result >> 31) == 1;
    storage->registers.PSTATE.Z = (result == 0);
    if (is_arithmetic) {
        if (is_addition) {
            storage->registers.PSTATE.C = result < a;
            storage->registers.PSTATE.V = ((int32_t)a > 0 && (int32_t)b > 0 && (int32_t)result < 0) ||
                        ((int32_t)a < 0 && (int32_t)b < 0 && (int32_t)result > 0);
        } else {
            storage->registers.PSTATE.C = a >= b;
            storage->registers.PSTATE.V = ((int32_t)a < 0 && (int32_t)b > 0 && (int32_t)result > 0) ||
                        ((int32_t)a > 0 && (int32_t)b < 0 && (int32_t)result < 0);
        }
    } else {
        storage->registers.PSTATE.C = false;
        storage->registers.PSTATE.V = false;
    }
}

uint64_t sign_extend(uint64_t value, int n_bits) {
    int64_t m = 1LL << (n_bits - 1);
    return (int64_t)(value ^ m) - m;
}

int memory_load_64(const Storage *storage, uint64_t address, uint64_t *value) {
    if (address + 7 >= MEMORY_SIZE) {
        fprintf(stderr, "Memory access out of bounds at address 0x%08" PRIx64 "\n", address);
        return 0;
    }

    memcpy(value, &storage->memory[address], sizeof(uint64_t));
    return 1;
}

int memory_load_32(const Storage *storage, uint64_t address, uint32_t *value) {
    if (address + 3 >= MEMORY_SIZE) {
        fprintf(stderr, "Memory access out of bounds at address 0x%08" PRIx64 "\n", address);
        return 0;
    }

    memcpy(value, &storage->memory[address], sizeof(uint32_t));
    return 1;
}

void memory_store_64(Storage *storage, uint64_t address, uint64_t value) {
    if (address + 7 >= MEMORY_SIZE) {
        fprintf(stderr, "Memory access out of bounds at address 0x%08" PRIx64 "\n", address);
        return;
    }

    memcpy(&storage->memory[address], &value, sizeof(value));
}

void memory_store_32(Storage *storage, uint64_t address, uint32_t value) {
    if (address + 3 >= MEMORY_SIZE) {
        fprintf(stderr, "Memory access out of bounds at address 0x%08" PRIx64 "\n", address);
        return;
    }

    memcpy(&storage->memory[address], &value, sizeof(value));
}

void apply_shift_64(uint8_t shift_type, uint8_t shift_amount, uint64_t *operand) {
    switch (shift_type) {
        // Logical shift left
        case 0x0: *operand = *operand << shift_amount; break;
        // Logical shift right
        case 0x1: *operand = *operand >> shift_amount; break;
        // Arithmetic shift right
        case 0x2: {
            int64_t signed_operand = (int64_t)(*operand);
            signed_operand = signed_operand >> shift_amount;
            *operand = (uint64_t)signed_operand;
            break;
        }
        // Rotate right
        default: {
            if (shift_amount != 0) {
                *operand = (*operand >> shift_amount) | (*operand << (64 - shift_amount));
            }
            break;
        }
    }
}

void apply_shift_32(uint8_t shift_type, uint8_t shift_amount, uint32_t *operand) {
    switch (shift_type) {
        // Logical shift left
        case 0x0: *operand = *operand << shift_amount; break;
        // Logical shift right
        case 0x1: *operand = *operand >> shift_amount; break;
        // Arithmetic shift right
        case 0x2: {
            int32_t signed_operand = (int32_t)(*operand);
            signed_operand = signed_operand >> shift_amount;
            *operand = (uint32_t)signed_operand;
            break;
        }
        // Rotate right 
        default: {
            if (shift_amount != 0) {
                *operand = (*operand >> shift_amount) | (*operand << (32 - shift_amount)); 
            }
            break;
        }
    }
}
