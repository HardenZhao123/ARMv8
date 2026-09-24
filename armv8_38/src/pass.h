#ifndef PASS_H
#define PASS_H

#include <stdbool.h>
#include <stdio.h>

#include "table.h"

/**
 * @brief It performs a two-pass assembly process with first pass building the symbol table
 *        and second pass assembling instructions and write binary output.
 *
 * @param n If false, perform the first pass (collect labels). If true, perform the second pass (assemble).
 * @param file_in The input assembly source file to read lines from.
 * @param file_out The output file to write assembled binary instructions to.
 * @param symtable A pointer to the symbol table that maps labels to their corresponding addresses.
 */
extern void pass(bool n, FILE *file_in, FILE *file_out, Table *symtable);

#endif
