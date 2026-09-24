#ifndef INSTRUCTIONS_H
#define INSTRUCTIONS_H

#include <stdint.h>
#include <stdbool.h>

#define halt 0x8a000000 // Halt instruction defined

/**
 * @brief Representing the high-level instruction types
 */
typedef enum {
    data_processing_imm, // Data Processing Instruction (Immediate)
    data_processing_reg, // Data Processing Instruction (Register)
    single_data_transfer,// Single Data Transfer
    load_literal,        // Load Literal
    branch,              // Branch
    special              // Special (used in assemble for .int)
} instr_type;

/**
 * @brief Enum representing the specific opcodes supported
 */
typedef enum {
    // Data Processing Instruction (Immediate)
    add,
    adds,
    sub,
    subs,
    movn,
    movz,
    movk,
    // Data Processing Instruction (Register)
    addr,
    addsr,
    subr,
    subsr,
    and_op,
    bic,
    orr,
    orn,
    eor,
    eon,
    ands,
    bics,
    madd,
    msub,
    // Single Data Transfer
    ldr,
    str,
    // Branch
    b,
    br,
    b_cond
} opcode;

/**
 * @brief Instruction structure representing a decoded ARMv8 instruction
 *
 * It contains:
 * - Raw 32-bit instruction
 * - Decoded instruction type and opcode identified
 * - Operand data
 */
typedef struct {
    uint32_t raw_instr; // Original 32-bit binary instruction
    instr_type type;    // Decoded instruction type
    opcode op;          // Decoded opcode
    
    union {
        // Data Processing Instruction (Immediate)
        struct {
            uint64_t operand;
            uint16_t imm16;
            uint16_t imm12:12;
            uint8_t rd:5;    
            uint8_t rn:5;
            uint8_t sf:1;
            uint8_t opi:3;
            uint8_t opc:2;
            uint8_t hw:2;
            uint8_t sh:1;
        } dp_imm;
        
        // Data Processing Instruction (Register)
        struct {
            uint8_t rd:5;    
            uint8_t rn:5;   
            uint8_t rm:5;           
            uint8_t sf:1;
            uint8_t operand:6;          
            uint8_t opr:4;  
            uint8_t opc:2;
            uint8_t shift:2;
            uint8_t M:1;  
            uint8_t ra:5;
            uint8_t x:1;
        } dp_reg;
        
        // Single Data Transfer
        struct {
            uint16_t offset:12; 
            uint8_t rt:5;     
            uint8_t xn:5;     
            uint8_t sf:1;    
            uint8_t L:1; 
        } sdt;

        // Load Literal
        struct {
            int64_t simm19;
            uint8_t rt:5;
            uint8_t sf:1;
        } load_l;

        // Branch
        struct {
            int64_t simm26;
            int64_t simm19;
            uint8_t cond;
            uint8_t xn;
        } branch;
    };
} Instruction;

/**
 * @brief Decodes a raw 32-bit instruction into a structured Instruction
 *
 * @param raw_instruction 32-bit binary ARMv8 instruction.
 * @param instr The pointer to the initial instruction to be returned after decoding.
 * @return Decoded Instruction structure defined in src/instruction.h
 */
extern bool decode_instruction(uint32_t raw_instruction, Instruction* instr);

#endif
