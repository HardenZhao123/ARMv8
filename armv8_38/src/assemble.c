#include <assert.h>
#include <stdlib.h>

#include "pass.h"

/**
 * @brief Assembles an input assembly file into binary output using a two-pass process.
 *
 * @param file_in The input file pointer to the assembly source code.
 * @param file_out The output file pointer to write assembled 32-bit binary encodings.
 */
void assemble(FILE *file_in, FILE *file_out) {
    Table *symtable = initialize_table();
    pass(0, file_in, file_out, symtable);
    pass(1, file_in, file_out, symtable);
    free_table(symtable);
}

/**
 *
 * @brief The main entry point of the assembler program.
 *
 * Opens the input source code file for reading and output file for writing binary encodings of instructions.
 * Calls the assemble function to process the input and output.
 */
int main(const int argc, const char **argv) {
    assert(argc == 3);

    // Open input file
    FILE *file_in = fopen(argv[1], "r");
    if (!file_in) {
        // If failed to open `file_in`
        perror(argv[1]);
        return EXIT_FAILURE;
    }

    // Open binary output file
    FILE *file_out = fopen(argv[2], "wb");
    if (!file_out) {
        // If failed to open `file_out`
        perror(argv[2]);
        return EXIT_FAILURE;
    }

    assemble(file_in, file_out);

    fclose(file_in);
    fclose(file_out);
    return EXIT_SUCCESS;
}
