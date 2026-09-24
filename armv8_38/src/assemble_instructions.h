#ifndef ASSEMBLE_INSTRUCTIONS_H
#define ASSEMBLE_INSTRUCTIONS_H

#include "intermediate_reps.h"

/**
 * @brief Assembles a single instruction using the intermediate representation.
 *
 * @param instr_rep The intermediate representation of the instruction.
 * @param instr_address The address of the instruction.
 * @param file_out The file to write the assembled instruction to.
 */
extern void assemble_instructions(const IntermediateReps* instr_rep, uint64_t instr_address, FILE* file_out);

#endif
