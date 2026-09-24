#include <stdio.h>

#include "assemble_instructions.h"

#define ERROR UINT32_MAX

/**
 * @brief Assembles a data processing instruction (immediate) to 32-bit binary encoding.
 *
 * @param instr_rep The intermediate format of the instruction to assemble.
 * @return The 32-bit binary encoding of this instruction.
 */
static uint32_t assemble_data_processing_imm(const IntermediateReps* instr_rep)
{
    uint32_t result = 0;
    opcode op = instr_rep->op;
    if (op == add || op == adds || op == sub || op == subs) // Arithmetic
    {
        int sf = instr_rep->arith_imm.is_64 ? 1 : 0;
        int sh = instr_rep->arith_imm.shift_left_12 ? 1 : 0;
        int rd = instr_rep->arith_imm.Rd;
        int rn = instr_rep->arith_imm.Rn;
        uint16_t imm12 = instr_rep->arith_imm.imm;
        uint8_t opc = 0;
        switch (op)
        {
            case add: opc = 0x0; break;
            case adds: opc = 0x1; break;
            case sub: opc = 0x2; break;
            default: opc = 0x3;
        }
        result = (sf << 31) | (opc << 29) | (0x4 << 26) | (0x2 << 23) |
                 (sh << 22) | ((imm12 & 0xFFF) << 10) |
                 ((rn & 0x1F) << 5) | (rd & 0x1F);
    } else // Wide Move
    {
        int sf = instr_rep->wide_move.is_64 ? 1 : 0;
        int rd = instr_rep->wide_move.Rd;
        uint16_t imm16 = instr_rep->wide_move.imm;
        uint8_t opc = 0;
        uint8_t hw = instr_rep->wide_move.shift_amount >> 4;
        switch (op)
        {
            case movn: opc = 0x0; break;
            case movz: opc = 0x2; break;
            case movk: opc = 0x3; break;
            default: break;
        }
        result = (sf << 31) | (opc << 29) | (0x4 << 26) | (0x5 << 23) |
                 ((hw & 0x3) << 21) | ((imm16 & 0xFFFF) << 5) | (rd & 0x1F);
    }

    return result;
}

/**
 * @brief Assembles a data processing instruction (register) to 32-bit binary encoding.
 *
 * @param instr_rep The intermediate format of the instruction to assemble.
 * @return The 32-bit binary encoding of this instruction.
 */
static uint32_t assemble_data_processing_reg(const IntermediateReps* instr_rep)
{
    uint32_t result = 0;
    opcode op = instr_rep->op;
    if (op == madd || op == msub) // Multiply
    {
        int sf = instr_rep->multiply.is_64 ? 1 : 0;
        int rm = instr_rep->multiply.Rm;
        int ra = instr_rep->multiply.Ra;
        int rn = instr_rep->multiply.Rn;
        int rd = instr_rep->multiply.Rd;
        int x = 0;
        switch (op)
        {
            case madd: x = 0x0; break;
            default: x = 0x1;
        }
        result = (sf << 31) | (0xd8 << 21) | ((rm & 0x1F) << 16) |
            (x << 15) | ((ra & 0x1F) << 10) | ((rn & 0x1F) << 5) | (rd & 0x1F);
    } else if (op == addr || op == addsr || op == subr || op == subsr) // Arithmetic
    {
        int rd = instr_rep->arith_reg.Rd;
        int rn = instr_rep->arith_reg.Rn;
        int rm = instr_rep->arith_reg.Rm;
        int sf = instr_rep->arith_reg.is_64 ? 1 : 0;
        uint8_t opc = 0;
        uint8_t shift_type = 0;
        uint8_t operand = instr_rep->arith_reg.shift_amount;
        switch (instr_rep->arith_reg.shift_type)
        {
            case lsl: shift_type = 0x0; break;
            case lsr: shift_type = 0x1; break;
            case asr: shift_type = 0x2; break;
            default: {
                fprintf(stderr, "Invalid data processing register instruction.\n");
                return ERROR;
            }
        }
        switch (op)
        {
            case addr: opc = 0x0; break;
            case addsr: opc = 0x1; break;
            case subr: opc = 0x2; break;
            default: opc = 0x3;
        }
        result = (sf << 31) | (opc << 29) | (0x0 << 28) | (0x5 << 25) | (0x1 << 24) |
                 (shift_type << 22) | (0x0 << 21) | ((rm & 0x1F) << 16) | ((operand & 0x3F) << 10) |
                 ((rn & 0x1F) << 5) | (rd & 0x1F);
    } else // Logical
    {
        int rd = instr_rep->bitlogic.Rd;
        int rn = instr_rep->bitlogic.Rn;
        int rm = instr_rep->bitlogic.Rm;
        int sf = instr_rep->bitlogic.is_64 ? 1 : 0;
        uint8_t opc = 0;
        uint8_t N = 0;
        uint8_t shift_type = 0;
        uint8_t operand = instr_rep->bitlogic.shift_amount;
        switch (instr_rep->bitlogic.shift_type)
        {
            case lsl: shift_type = 0x0; break;
            case lsr: shift_type = 0x1; break;
            case asr: shift_type = 0x2; break;
            case ror: shift_type = 0x3; break;
        }
        switch (op)
        {
        case and_op: opc = 0x0; N = 0x0; break;
        case bic: opc = 0x0; N = 0x1; break;
        case orr: opc = 0x1; N = 0x0; break;
        case orn: opc = 0x1; N = 0x1; break;
        case eor: opc = 0x2; N = 0x0; break;
        case eon: opc = 0x2; N = 0x1; break;
        case ands: opc = 0x3; N = 0x0; break;
        case bics: opc = 0x3; N = 0x1; break;
        default: break;
        }
        result = (sf << 31) | (opc << 29) | (0x0 << 28) | (0x5 << 25) | (0x0 << 24) |
                 (shift_type << 22) | (N << 21) | ((rm & 0x1F) << 16) | ((operand & 0x3F) << 10) |
                 ((rn & 0x1F) << 5) | (rd & 0x1F);
    }

    return result;
}

/**
 * @brief Assembles a single data transfer or load literal instruction to 32-bit binary encoding.
 *
 * @param instr_rep The intermediate format of the instruction to assemble.
 * @param instr_address The address of this instruction.
 * @return The 32-bit binary encoding of this instruction.
 */
static uint32_t assemble_load_store(const IntermediateReps* instr_rep, uint64_t instr_address) {
    uint32_t instr = 0;
    uint16_t offset = 0;
    uint32_t simm19 = 0;
    uint16_t mask12 = (1U << 12) - 1;
    uint8_t I;
    uint8_t sf = (instr_rep->load_store.is_64) ? 1 : 0;
    LoadStoreType ls_type = instr_rep->load_store.ls_type;
    switch (ls_type) {
        case ls_signed_pre: {
            uint16_t simm9 = ((1U << 9) - 1) & instr_rep->load_store.simm;
            I = 1;
            offset = mask12 & (simm9 << 2 | I << 1 | 1);
            break;
        }
        case ls_signed_post: {
            uint16_t simm9 = ((1U << 9) - 1) & instr_rep->load_store.simm;
            I = 0;
            offset = mask12 & (simm9 << 2 | I << 1 | 1);
            break;
        }
        // unsigned offset or zero unsigned offset
        case ls_unsigned: {
            offset = mask12 & ((sf == 1) ? (instr_rep->load_store.imm >> 3) : (instr_rep->load_store.imm >> 2));
            break;
        }
        case ls_register: {
            offset = mask12 & (1 << 11 | instr_rep->load_store.Xm << 6 | 0x1a);
            break;
        }
        // default case for ls_literal
        default: {
            uint32_t mask19 = (1U << 19) - 1;
            simm19 = mask19 & (((int64_t)(instr_rep->load_store.label_address - instr_address)) >> 2);
            break;
        }
    }
    uint8_t mask5 = (1U << 5) - 1;
    uint8_t Rt = mask5 & instr_rep->load_store.Rt;
    uint8_t Xn = mask5 & instr_rep->load_store.Xn;
    if (ls_type == ls_literal) {
        instr = sf << 30 | 0x18 << 24 | simm19 << 5 | Rt;
    } else {
        uint8_t U = (ls_type == ls_unsigned) ? 1 : 0;
        uint8_t L = (instr_rep->op == ldr) ? 1 : 0;
        instr = 1 << 31 | sf << 30 | 0x1c << 25 | U << 24 | L << 22 | offset << 10 | Xn << 5 | Rt;
    }
    return instr;
}

/**
 * @brief Assembles a branch instruction to 32-bit binary encoding.
 *
 * @param instr_rep The intermediate format of the instruction to assemble.
 * @param instr_address The address of this instruction.
 * @return The 32-bit binary encoding of this instruction.
 */
static uint32_t assemble_branch(const IntermediateReps* instr_rep, uint64_t instr_address) {
    uint32_t instr = 0;
    switch (instr_rep->op) {
        case b: {
            uint32_t simm26 = 0;
            uint32_t mask26 = (1U << 26) - 1;
            simm26 = mask26 & (((int64_t) instr_rep->branch_address - instr_address) >> 2);
            instr = 0x5 << 26 | simm26;
            break;
        }
        case br: {
            uint8_t mask5 = (1U << 5) - 1;
            uint8_t Xn = mask5 & instr_rep->branch_reg;
            instr = 0x3587c0 << 10 | Xn << 5;
            break;
        }
        case b_cond: {
            uint32_t simm19 = 0;
            uint32_t mask19 = (1U << 19) - 1;
            uint8_t mask4 = (1U << 4) - 1;
            simm19 = mask19 & (((int64_t) instr_rep->branch_cond.branch_address - instr_address) >> 2);
            instr = 0x54 << 24 | simm19 << 5 | (mask4 & instr_rep->branch_cond.cond);
            break;
        }
        default: {
            fprintf(stderr, "Invalid branch instruction\n");
            return ERROR;
        }
    }
    return instr;
}

/**
 * @brief Writes a 32-bit encoding of instruction to a binary file.
 *
 * @param file_out The file to write the assembled instruction to.
 * @param binary_instr The 32-bit binary encoding of this instruction to write into the file.
 */
static void write_single_instr(FILE* file_out, uint32_t binary_instr)
{
    if (fwrite(&binary_instr, sizeof(binary_instr), 1, file_out) != 1)
    {
        perror("Failed to write in file.\n");
    }
}

void assemble_instructions(const IntermediateReps* instr_rep, uint64_t instr_address, FILE* file_out) {
    uint32_t result = 0;
    switch (instr_rep->type)
    {
        case data_processing_imm:
            result = assemble_data_processing_imm(instr_rep);
            break;
        case data_processing_reg:
            result = assemble_data_processing_reg(instr_rep);
            break;
        case single_data_transfer:
        case load_literal:
            result = assemble_load_store(instr_rep, instr_address);
            break;
        case branch:
            result = assemble_branch(instr_rep, instr_address);
            break;
        // instr_rep->type == special
        default:
            result = (uint32_t) instr_rep->special_imm;
    }
    write_single_instr(file_out, result);
}
