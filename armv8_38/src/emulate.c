#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>

#include "pipeline.h"
#include "storage.h"

/**
 * @brief Outputs the current state of the machine
 * 
 * It includes:
 * - The contents of all registers
 * - The contents of Program Counter
 * - The PSTATE flags (N, Z, C, V)
 * - All contents of non-zero memory (4-byte alligned)
 */
static void output_state(FILE *out, const Storage *storage) {
    // Registers
    fprintf(out, "Registers:\n");
    for (int i = 0; i < NUM_GENERAL_PURPOSE_REGISTERS; i++)
        fprintf(out, "X%02d    = %016" PRIx64 "\n", i, storage->registers.R[i]);
    // PC
    fprintf(out, "PC     = %016" PRIx64 "\n", storage->registers.PC);
    // PSTATE (conditional flags)
    fprintf(out, "PSTATE : %c%c%c%c\n",
            storage->registers.PSTATE.N ? 'N' : '-',
            storage->registers.PSTATE.Z ? 'Z' : '-',
            storage->registers.PSTATE.C ? 'C' : '-',
            storage->registers.PSTATE.V ? 'V' : '-');

    // Non-zero memory
    fprintf(out, "Non-Zero Memory:\n");
    for (size_t addr = 0; addr < MEMORY_SIZE; addr += 4) {
        const uint32_t value = *(uint32_t *) (storage->memory + addr);
        if (value)
            fprintf(out, "0x%08zx: %08" PRIx32"\n", addr, value);
    }
}

/**
 * @brief Main entry point for the emulator
 * 
 * It contains:
 * - Loads binary memory from input file
 * - Executes instructions until HALT is encountered
 * - Writes final state to output (stdout by default)
 */
int main(const int argc, char *argv[]) {
    if (argc < 2 || argc > 3) {
        // Invalid number of arguments
        fprintf(stderr, "Usage: %s <input_file> [<output_file>]\n", argv[0]);
        return EXIT_FAILURE;
    }

    // Open binary input file
    FILE *file_in = fopen(argv[1], "rb");
    if (!file_in) {
        // If failed to open `file_in`
        perror(argv[1]);
        return EXIT_FAILURE;
    }

    // Set up output stream (`file_out` defaults to `stdout`)
    FILE *file_out = stdout;
    if (argv[2]) {
        // `file_out` is specified
        file_out = fopen(argv[2], "w");
        if (!file_out) {
            // If failed to open `file_out`
            perror(argv[2]);
            return EXIT_FAILURE;
        }
    }

    // Initialize storage and load memory
    Storage storage = empty_storage();
    // Initialize memory with binary from `file_in`
    fread(&storage.memory, 1, sizeof(storage.memory), file_in);
    if (ferror(file_in)) {
        // If failed to read from `file_in`
        perror(argv[1]);
        return EXIT_FAILURE;
    }
    fclose(file_in);

    // Execute instructions
    int emulate_output = emulate(&storage);
    if (emulate_output == -1) {
        // If we failed to emulate
        perror("Failed to emulate");
        return EXIT_FAILURE;
    }

    // Output final state
    output_state(file_out, &storage);
    fclose(file_out);
    return EXIT_SUCCESS;
}
