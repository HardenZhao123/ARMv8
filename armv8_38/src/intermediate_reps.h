#ifndef INTERMEDIATE_REPS_H
#define INTERMEDIATE_REPS_H

#include "instructions.h"

/**
 * @brief Representing the type of shift
 */
typedef enum {
    lsl,
    lsr, 
    asr, 
    ror
} ShiftType;

/**
 * @brief Representing the type of load/store instruction
 */
typedef enum {
    ls_signed_pre,
    ls_signed_post,
    ls_unsigned,
    ls_register,
    ls_literal
} LoadStoreType;

/**
 * @brief The intermediate structure of an instruction after tokenising and parsing
 *
 * It contains:
 * - Type of instruction
 * - Type of opcode
 * - The union of instruction formats after tokenising and parsing
 */
typedef struct {
    instr_type type;
    opcode op;

    // operands for instructions
    union {
        struct {
            // add, adds, sub, subs, neg, negs, cmp, cmn (immediate)
            uint16_t imm;
            int Rd;
            int Rn;
            // true if instruction has optional operands
            bool has_optional;
            // true if instruction is 64-bit
            bool is_64;
            bool shift_left_12;
        } arith_imm;

        // add, adds, sub, subs, neg, negs, cmp, cmn (register)
        struct {
            int Rd;
            int Rn;
            int Rm;
            bool is_64;
            bool has_optional;
            uint8_t shift_amount;
            ShiftType shift_type;
        } arith_reg;

        // and, ands, bic, bics, eor, orr, eon, orn, tst, mvn, mov
        struct {
            int Rd;
            int Rn;
            int Rm;
            ShiftType shift_type;
            uint8_t shift_amount;
            bool has_optional;
            bool is_64;
        } bitlogic;

        // movk, movn, movz
        struct {
            int Rd;
            uint16_t imm;
            uint8_t shift_amount;
            bool has_optional;
            bool is_64;
        } wide_move;

        // madd, msub, mul, mneg
        struct {
            int Rd;
            int Rn;
            int Rm;
            int Ra;
            bool is_64;
        } multiply;

        // b
        uint64_t branch_address;

        // b.cond
        struct {
            uint64_t branch_address;
            uint8_t cond;
        } branch_cond;
        
        // br
        int branch_reg;

        // str, ldr
        struct {
            int Rt;
            int Xn;
            LoadStoreType ls_type;
            int Xm;
            int16_t simm;
            uint16_t imm;
            bool is_64;
            uint64_t label_address;
        } load_store;

        // .int (special)
        int32_t special_imm;
    };
} IntermediateReps;

#endif
