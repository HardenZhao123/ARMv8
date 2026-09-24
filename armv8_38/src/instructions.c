#include <stdio.h>

#include "instructions.h"
#include "utilities.h"

bool decode_instruction(uint32_t raw_instruction, Instruction* instr) {
    // Initialize a new Instruction instr
    instr->raw_instr = raw_instruction;
    // Data Processing Instruction (Immediate)
    if (extract_bits(raw_instruction, 28, 26) == 0x4) {
        instr->type = data_processing_imm;
        instr->dp_imm.rd = extract_bits(raw_instruction, 4, 0);
        instr->dp_imm.rn = extract_bits(raw_instruction, 9, 5);
        instr->dp_imm.sf = extract_bits(raw_instruction, 31, 31);
        instr->dp_imm.opi = extract_bits(raw_instruction, 25, 23);
        instr->dp_imm.opc = extract_bits(raw_instruction, 30, 29);
        uint8_t opc = instr->dp_imm.opc;
        uint8_t opi = instr->dp_imm.opi;
        if (opi == 0x2) {
            // Arithmetic
            instr->dp_imm.sh = extract_bits(raw_instruction, 22, 22);
            instr->dp_imm.imm12 = extract_bits(raw_instruction, 21, 10);
            instr->dp_imm.operand = // shift left 12 bits if sh = 1
                (uint64_t)instr->dp_imm.imm12 << ((instr->dp_imm.sh == 1) ? 12 : 0);
            switch (opc) {
                case 0x0: instr->op = add; break;
                case 0x1: instr->op = adds; break;
                case 0x2: instr->op = sub; break;
                default: instr->op = subs;
            }
        } else if (opi == 0x5) {
            // Wide Move
            instr->dp_imm.hw = extract_bits(raw_instruction, 22, 21);
            instr->dp_imm.imm16 = extract_bits(raw_instruction, 20, 5);
            instr->dp_imm.operand = // imm16 << (hw * 16)
                (uint64_t)instr->dp_imm.imm16 << (instr->dp_imm.hw * 16);
            switch (opc) {
                case 0x0: instr->op = movn; break;
                case 0x2: instr->op = movz; break;
                case 0x3: instr->op = movk; break;
                default:
                    fprintf(stderr, "Invalid wide move opc: %u\n", opc);
                    return false;
            }
        } else {
            fprintf(stderr, "Unsupported Data Processing Immediate opi: %u\n", opi);
            return false;
        }
    }
    // Data Processing Instruction (Register)
    else if (extract_bits(raw_instruction, 27, 25) == 0x5) {
        instr->type = data_processing_reg;
        instr->dp_reg.rd = extract_bits(raw_instruction, 4, 0);
        instr->dp_reg.rn = extract_bits(raw_instruction, 9, 5);
        instr->dp_reg.operand = extract_bits(raw_instruction, 15, 10);
        instr->dp_reg.rm = extract_bits(raw_instruction, 20, 16);
        instr->dp_reg.opr = extract_bits(raw_instruction, 24, 21);
        instr->dp_reg.M = extract_bits(raw_instruction, 28, 28);
        instr->dp_reg.opc = extract_bits(raw_instruction, 30, 29);
        instr->dp_reg.sf = extract_bits(raw_instruction, 31, 31);
        if (instr->dp_reg.M == 0) {
            // Arithmetic or Logical operation
            uint8_t opc = instr->dp_reg.opc;
            instr->dp_reg.shift = extract_bits(raw_instruction, 23, 22);
            if (extract_bits(raw_instruction, 24, 24) == 1) {
                // Arithmetic
                switch (opc) {
                    case 0x0: instr->op = addr; break;
                    case 0x1: instr->op = addsr; break;
                    case 0x2: instr->op = subr; break;
                    default: instr->op = subsr;
                }
            } else {
                // Logical
                uint8_t N = extract_bits(raw_instruction, 21, 21);
                switch ((opc << 1) | N) {
                    case 0x0: instr->op = and_op; break;
                    case 0x1: instr->op = bic; break;
                    case 0x2: instr->op = orr; break;
                    case 0x3: instr->op = orn; break;
                    case 0x4: instr->op = eor; break;
                    case 0x5: instr->op = eon; break;
                    case 0x6: instr->op = ands; break;
                    default: instr->op = bics;
                }
            }
        } else if (instr->dp_reg.M == 1 && instr->dp_reg.opr == 0x8) {
            // Multiply
            instr->dp_reg.ra = extract_bits(raw_instruction, 14, 10);
            uint8_t x = extract_bits(raw_instruction, 15, 15);
            instr->op = (x == 0) ? madd : msub;
        } else {
            fprintf(stderr, "Unsupported Data Processing Register type: M=%u, opr=%u\n", instr->dp_reg.M, instr->dp_reg.opr);
            return false;
        }
    }
    // Single Data Transfer & Load Literal
    else if (extract_bits(raw_instruction, 28, 25) == 0xc) {
        uint8_t msb_raw = extract_bits(raw_instruction, 31, 31);
        if (msb_raw == 0x1) {
            // Single Data Transfer (Load/Store)
            instr->type = single_data_transfer;
            instr->sdt.sf = extract_bits(raw_instruction, 30, 30); // // 0 = 32-bit, 1 = 64-bit
            instr->sdt.L = extract_bits(raw_instruction, 22, 22); // 1 = load, 0 = store
            instr->sdt.offset = extract_bits(raw_instruction, 21, 10);
            instr->sdt.xn = extract_bits(raw_instruction, 9, 5);
            instr->sdt.rt = extract_bits(raw_instruction, 4, 0);
            instr->op = (instr->sdt.L == 1) ? ldr : str;
        } else if (msb_raw == 0x0) {
            // Load Literal
            instr->type = load_literal;
            instr->load_l.sf = extract_bits(raw_instruction, 30, 30);
            instr->load_l.simm19 = (int64_t)extract_bits_signed(raw_instruction, 23, 5);
            instr->load_l.rt = extract_bits(raw_instruction, 4, 0);
            instr->op = ldr;
        }
    }
    // Branch Instruction
    else if (extract_bits(raw_instruction, 28, 26) == 0x5) {
        instr->type = branch;
        // Unconditional branch
        if (extract_bits(raw_instruction, 31, 26) == 0x5) {
            instr->branch.simm26 = (int64_t) extract_bits_signed(raw_instruction, 25, 0);
            instr->op = b;
        }
        // Conditional branch
        else if (extract_bits(raw_instruction, 31, 24) == 0x54) {
            instr->branch.simm19 = (int64_t) extract_bits_signed(raw_instruction, 23, 5);
            instr->branch.cond = extract_bits(raw_instruction, 3, 0);
            instr->op = b_cond;
        }
        // Register branch
        else if (extract_bits(raw_instruction, 31, 24) == 0xd6) {
            instr->branch.xn = extract_bits(raw_instruction, 9, 5);
            instr->op = br;
        } else {
            fprintf(stderr, "Unsupported Branch Instruction type.\n");
            return false;
        }
    }
    // Invalid Instruction
    else {
        fprintf(stderr, "Invalid Instruction.\n");
        return false;
    }
    return true;
}
