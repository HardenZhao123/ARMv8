#include <assert.h>
#include <stdio.h>

#include "utilities.h"
#include "instructions.h"

/**
 * @brief Execute a Data Processing Immediate instruction.
 * 
 * It handles add, adds, sub, subs, movn, movz, movk.
 */
static void execute_dp_imm(const Instruction *instr, Storage *storage) {
    switch (instr->op) {
        case adds:
        case add: {
            uint64_t operand2 = instr->dp_imm.operand;

            // Check for sf flag (64/32-bit operation)
            if (instr->dp_imm.sf == 1) {
                // 64-bit
                uint64_t operand1 = get_x(&storage->registers, instr->dp_imm.rn);
                uint64_t result = operand1 + operand2;
                set_x(&storage->registers, instr->dp_imm.rd, result);
                if (instr->op == adds) {
                    update_flags_64b(storage, result, operand1, operand2, true, true);
                }
            } else { 
                // 32-bit
                uint32_t operand1 = get_w(&storage->registers, instr->dp_imm.rn);
                uint32_t result = operand1 + (uint32_t)operand2;
                set_w(&storage->registers, instr->dp_imm.rd, result);
                if (instr->op == adds) {
                    update_flags_32b(storage, result, operand1, (uint32_t)operand2, true, true);
                }
            }
            break;
        }
        case subs:
        case sub: {
            uint64_t operand2 = instr->dp_imm.operand;

            // Check for sf flag (64/32-bit operation)
            if (instr->dp_imm.sf == 1) {
                // 64-bit
                uint64_t operand1 = get_x(&storage->registers, instr->dp_imm.rn);
                uint64_t result = operand1 - operand2;
                set_x(&storage->registers, instr->dp_imm.rd, result);
                if (instr->op == subs) {
                    update_flags_64b(storage, result, operand1, operand2, false, true);
                }
            } else {
                // 32-bit
                uint32_t operand1 = get_w(&storage->registers, instr->dp_imm.rn);
                uint32_t result = operand1 - (uint32_t)operand2;
                set_w(&storage->registers, instr->dp_imm.rd, result);
                if (instr->op == subs) {
                    update_flags_32b(storage, result, operand1, (uint32_t)operand2, false, true);
                }
            }
            break;
        }
        case movn: { // Move NOT (inverted operand value into register)
            // Check for sf flag (64/32-bit operation)
            if (instr->dp_imm.sf == 1) {
                set_x(&storage->registers, instr->dp_imm.rd, ~instr->dp_imm.operand);
            } else {
                // Check for valid shift value for a 32-bit operation
                if (instr->dp_imm.hw > 1) {
                    fprintf(stderr, "invalid shift value for a 32-bit operation\n");
                    return;
                }
                set_w(&storage->registers, instr->dp_imm.rd, ~(uint32_t)instr->dp_imm.operand);
            }
            break;
        }
        case movz: { // Move zero (zero the rest of register and insert operand)
            // Check for sf flag (64/32-bit operation)
            if (instr->dp_imm.sf == 1) {
                set_x(&storage->registers, instr->dp_imm.rd, instr->dp_imm.operand);
            } else {
                // Check for valid shift value for a 32-bit operation
                if (instr->dp_imm.hw > 1) {
                    fprintf(stderr, "invalid shift value for a 32-bit operation\n");
                    return;
                }
                set_w(&storage->registers, instr->dp_imm.rd, (uint32_t)instr->dp_imm.operand);
            }
            break;
        }
        case movk: { // Move keep (replace 16-bit field in destination register)
            // Check for sf flag (64/32-bit operation)
            if (instr->dp_imm.sf == 1) {
                uint64_t rd_val = get_x(&storage->registers, instr->dp_imm.rd);

                // Create a mask with zeros in the target 16-bit field
                uint64_t mask = ~(UINT64_C(0xFFFF) << (instr->dp_imm.hw * 16));

                // Clear the target bits and set the bits from imm16
                rd_val = (rd_val & mask) | instr->dp_imm.operand;
                set_x(&storage->registers, instr->dp_imm.rd, rd_val);
            } else {
                // Check for valid shift value for a 32-bit operation
                if (instr->dp_imm.hw > 1) {
                    fprintf(stderr, "invalid shift value for a 32-bit operation\n");
                    return;
                }
                uint32_t rd_val = get_w(&storage->registers, instr->dp_imm.rd);

                // Create a mask with zeros in the target 16-bit field
                uint32_t mask = ~(UINT32_C(0xFFFF) << (instr->dp_imm.hw * 16));

                // Clear the target bits and set the bits from imm16
                rd_val = (rd_val & mask) | (uint32_t)instr->dp_imm.operand;
                set_w(&storage->registers, instr->dp_imm.rd, rd_val);
            }
            break;
        }
        default:
            fprintf(stderr, "invalid instruction\n");
            break;
    }
}

/**
 * @brief Execute a single data transfer instruction
 * 
 * It handles ldr, str.
 */
static void execute_single_data_transfer(const Instruction *instr, Storage *storage) {
    int isLoad = instr->sdt.L;
    int is_64_bit = instr->sdt.sf;

    uint32_t rt = instr->sdt.rt;
    uint64_t address = 0;
    uint8_t xn = extract_bits(instr->raw_instr, 9, 5);
    uint64_t base = get_x(&storage->registers, xn); // The value stored in the register xn (the base register).

    int U = extract_bits(instr->raw_instr, 24, 24);
    int is_register_offset = extract_bits(instr->raw_instr, 21, 21);

    if (U) {
        // Unsigned immediate offset (Address = Xn + uoffset)
        uint64_t imm12 = extract_bits(instr->raw_instr, 21, 10);
        uint64_t uoffset = ((instr->sdt.sf) ? (8 * imm12) : (4 * imm12));
        address = uoffset + base;
    } else if (is_register_offset) {
        // Register offset (Address = Xn + Xm)
        uint64_t xm = get_x(&storage->registers, extract_bits(instr->raw_instr, 20, 16));
        address = base + xm;
    } else {
        // Pre/Post-Index offset
        int is_pre_index = extract_bits(instr->raw_instr, 11, 11);
        int64_t simm9 = extract_bits_signed(instr->raw_instr, 20, 12);
        if (is_pre_index) {
            // Pre-Indexed (Address = Xn + simm9 and Xn := Xn + simm9 for write back)
            address = base + simm9;
            set_x(&storage->registers, xn, address);
        } else {
            // Post-Indexed (Address = Xn and Xn := Xn + simm9 for write back)
            address = base;
            set_x(&storage->registers, xn, base + simm9);
        }
    }

    if (is_64_bit) {
        if (isLoad) {
            // 64-bit load
            uint64_t value;
            if (!memory_load_64(storage, address, &value)) {
                return;
            }
            set_x(&storage->registers, rt, value);
        } else {
            // 64-bit store
            uint64_t value = get_x(&storage->registers, rt);
            memory_store_64(storage, address, value);
        }
    } else {
        if (isLoad) {
            // 32-bit load
            uint32_t value;
            if (!memory_load_32(storage, address, &value)) {
                return;
            }
            set_w(&storage->registers, rt, value);
        } else {
            // 32-bit store
            uint32_t value = get_w(&storage->registers, rt);
            memory_store_32(storage, address, value);
        }
    }
}

/**
 * @brief Execute load literal instruction (Address = PC + simm19 * 4).
 */
static void execute_load_literal(const Instruction *instr, Storage *storage) {
    uint64_t address = 0;
    uint64_t pc = storage->registers.PC;
    address = pc + (instr->load_l.simm19 * 4);
    uint8_t rt = instr->load_l.rt;
    int is_64_bit = instr->load_l.sf;

    if (is_64_bit) {
        uint64_t value;
        if (!memory_load_64(storage, address, &value)) {
            return;
        }
        set_x(&storage->registers, rt, value);
    } else {
        uint32_t value;
        if (!memory_load_32(storage, address, &value)) {
            return;
        }
        set_w(&storage->registers, rt, value);
    }
}

/**
 * @brief Execute a data-processing register instruction
 * 
 * It handles arithmetic, logical and multiply instructions based on flags and opcodes.
 */
static void execute_dp_reg(const Instruction *instr, Storage *storage) {
    if (instr->op == madd || instr->op == msub) {
        // Check for sf flag (64/32-bit operation)
        if (instr->dp_reg.sf == 1) {
            uint64_t result;
            switch (instr->op) {
                case madd:
                    result =
                        get_x(&storage->registers, instr->dp_reg.ra) +
                        get_x(&storage->registers, instr->dp_reg.rn) *
                        get_x(&storage->registers, instr->dp_reg.rm);
                    break;
                case msub:
                    result =
                        get_x(&storage->registers, instr->dp_reg.ra) -
                        get_x(&storage->registers, instr->dp_reg.rn) *
                        get_x(&storage->registers, instr->dp_reg.rm);
                    break;
                default:
                    fprintf(stderr, "invalid instruction\n");
                    return;
            }
            set_x(&storage->registers, instr->dp_reg.rd, result);
        } else {
            uint32_t result;
            switch (instr->op) {
                case madd:
                    result =
                        get_w(&storage->registers, instr->dp_reg.ra) +
                        get_w(&storage->registers, instr->dp_reg.rn) *
                        get_w(&storage->registers, instr->dp_reg.rm);
                    break;
                case msub:
                    result =
                        get_w(&storage->registers, instr->dp_reg.ra) -
                        get_w(&storage->registers, instr->dp_reg.rn) *
                        get_w(&storage->registers, instr->dp_reg.rm);
                    break;
                default:
                    fprintf(stderr, "invalid instruction\n");
                    return;
            }
            set_w(&storage->registers, instr->dp_reg.rd, result);
        }
    } else {
    // Check for sf flag (64/32-bit operation)
        if (instr->dp_reg.sf == 1) {
            uint64_t operand2 = get_x(&storage->registers, instr->dp_reg.rm);
            uint64_t result;
            uint64_t rn_val = get_x(&storage->registers, instr->dp_reg.rn);

            // Apply shift
            apply_shift_64(instr->dp_reg.shift, instr->dp_reg.operand, &operand2);

            // Specific operations
            switch (instr->op) {
                case addr:   result = rn_val + operand2; break;
                case addsr:  {
                    result = rn_val + operand2;
                    update_flags_64b(storage, result, rn_val, operand2, true, true);
                    break;
                }
                case subr:   result = rn_val - operand2; break;
                case subsr:  {
                    result = rn_val - operand2;
                    update_flags_64b(storage, result, rn_val, operand2, false, true);
                    break;
                }
                case and_op: result = rn_val & operand2; break;
                case bic:    result = rn_val & ~operand2; break;
                case orr:    result = rn_val | operand2; break;
                case orn:    result = rn_val | ~operand2; break;
                case eor:    result = rn_val ^ operand2; break;
                case eon:    result = rn_val ^ ~operand2; break;
                case ands:
                    result = rn_val & operand2;
                    update_flags_64b(storage, result, rn_val, operand2, true, false);
                    break;
                case bics:
                    result = rn_val & ~operand2;
                    update_flags_64b(storage, result, rn_val, operand2, true, false);
                    break;
                default:
                    fprintf(stderr, "invalid instruction\n");
                    return;
            }
            set_x(&storage->registers, instr->dp_reg.rd, result);
        }
        else {
            uint32_t operand2 = get_w(&storage->registers, instr->dp_reg.rm);
            uint32_t result;
            uint32_t rn_val = get_w(&storage->registers, instr->dp_reg.rn);

            // Apply shift
            apply_shift_32(instr->dp_reg.shift, instr->dp_reg.operand, &operand2);

            switch (instr->op) {
                case addr:   result = rn_val + operand2; break;
                case addsr:  {
                    result = rn_val + operand2;
                    update_flags_32b(storage, result, rn_val, operand2, true, true);
                    break;
                }
                case subr:   result = rn_val - operand2; break;
                case subsr:  {
                    result = rn_val - operand2;
                    update_flags_32b(storage, result, rn_val, operand2, false, true);
                    break;
                }
                case and_op: result = rn_val & operand2; break;
                case bic:    result = rn_val & ~operand2; break;
                case orr:    result = rn_val | operand2; break;
                case orn:    result = rn_val | ~operand2; break;
                case eor:    result = rn_val ^ operand2; break;
                case eon:    result = rn_val ^ ~operand2; break;
                case ands:
                    result = rn_val & operand2;
                    update_flags_32b(storage, result, rn_val, operand2, true, false);
                    break;
                case bics:
                    result = rn_val & ~operand2;
                    update_flags_32b(storage, result, rn_val, operand2, true, false);
                    break;
                default:
                    fprintf(stderr, "invalid instruction\n");
                    return;
            }
            set_w(&storage->registers, instr->dp_reg.rd, result);
        }
    }
}

/**
 * @brief Evaluate whether a branch condition is met
 * 
 * @param storage The system state (for checking PSTATE flags)
 * @param encoding The 4-bit condition code
 * @return true if the branch condition is satisfied
 */
static inline bool branch_conditions(const Storage *storage, const uint8_t encoding) {
    bool Z = storage->registers.PSTATE.Z;
    bool N = storage->registers.PSTATE.N;
    bool V = storage->registers.PSTATE.V;

    switch (encoding) {
        case 0x0: return Z; // EQ
        case 0x1: return !Z; // NE
        case 0xa: return N == V; // GE
        case 0xb: return N != V; // LT
        case 0xc: return !Z && N == V; // GT
        case 0xd: return !(!Z && N == V); // LE
        case 0xe: return true; // AL
        default: return false;
    }
}

/**
 * @brief Execute a branch instruction
 * 
 * It handles:
 * - Unconditional branch (`b`)
 * - Register branch (`br`)
 * - Conditional branch (`b.cond`)
 */
static void execute_branch(const Instruction *instr, Storage *storage) {
    switch (instr->op) {
        case b: { // Branch to the address encoded by literal. PC ∶= PC + offset
                storage->registers.PC = storage->registers.PC + (uint64_t) (instr->branch.simm26 * 4 - 4);
                 // Unconditional += 4 on PC at the end of loop, subtract 4 here to offset to compensate
                break;
            }
        case br: { // Branch to the address in Xn. PC ∶= Xn
                storage->registers.PC = get_x(&storage->registers, instr->branch.xn) - 4;
                break;
            }
        default: { // Branch to literal only when PSTATE satisfies cond. If cond, PC ∶= PC + offset
                if (branch_conditions(storage, instr->branch.cond)) {
                    storage->registers.PC =
                        storage->registers.PC + (uint64_t) (instr->branch.simm19 * 4 - 4);
                }
            }
    }
}

void execute_instruction(const Instruction *instr, Storage *storage) {
    switch (instr->type) {
        case data_processing_imm:
            execute_dp_imm(instr, storage);
            break;
        case data_processing_reg:
            execute_dp_reg(instr, storage);
            break;
        case load_literal:
            execute_load_literal(instr, storage);
            break;
        case single_data_transfer:
            execute_single_data_transfer(instr, storage);
            break;
        case branch:
            execute_branch(instr, storage);
            break;
        default:
            assert(0);
    }
}

int emulate(Storage *storage) {
    int c;
    for (c = 0; ; c++) {
        // Create a new instruction
        Instruction instr = {0};
        // Fetch
        const uint32_t raw_instruction = *(uint32_t *) (storage->memory + storage->registers.PC);
        if (raw_instruction == halt) break;
        // Decode
        bool decode_success = decode_instruction(raw_instruction, &instr);
        if (!decode_success) {
            return -1;
        }
        // Execute
        execute_instruction(&instr, storage);
        storage->registers.PC += 4;
    }
    return c;
}
