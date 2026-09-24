#ifndef PARSER_H
#define PARSER_H

#include "intermediate_reps.h"
#include "table.h"

/**
 *
 * @param line The input string line to parse (An instruction or label).
 * @param symbols A pointer to the symbol table that maps each label to their corresponding addresses.
 * @param out Output parameter: the intermediate formats of that instruction.
 * @return true if line can be parsed into a valid instruction; otherwise false.
 */
bool parse_line(const char *line, Table *symbols, IntermediateReps *out);

#endif
