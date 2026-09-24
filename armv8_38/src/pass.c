#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "assemble_instructions.h"
#include "parser.h"

#define _POSIX_C_SOURCE 200809L

/**
 *
 * @param c The first character of a string line to check.
 * @return True if it is a valid first char; otherwise false.
 */
static bool isValidFirstChar(const char c) {
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || c == '_' ||
           c == '.';
}

/**
 *
 * @param c The character to check.
 * @return True if it is a digit; otherwise false.
 */
static bool isDigit(const char c) { return c >= '0' && c <= '9'; }

/**
 *
 * @param s The string line to check.
 * @return True if the string is a valid label; otherwise false.
 */
static bool isLabel(const char *s) {
    if (!isValidFirstChar(*s))
        return false;
    while (*++s)
        if (!isValidFirstChar(*s) && !isDigit(*s) && *s != '$')
            return false;
    return true;
}

/**
 *
 * @param str The string to trim.
 * @return A pointer to the trimmed string.
 */
static char *trim(char *str) {
    char *trimmed = str;
    size_t len = strlen(trimmed);
    while (len && isspace(*trimmed)) {
        trimmed++;
        len--;
    }
    while (len && isspace(trimmed[len - 1]))
        trimmed[--len] = '\0';
    return trimmed;
}

/**
 *
 * @param line The string line from input source file.
 * @param in_block_comment True if there is a block comment, otherwise false.
 * @return The string line without comment.
 */
static char *remove_comments(const char *line, bool *in_block_comment) {
    static char buffer[4096];
    size_t i = 0, j = 0;

    while (line[i] != '\0') {
        if (*in_block_comment) {
            if (line[i] == '*' && line[i+1] == '/') {
                *in_block_comment = false;
                i += 2;
            } else {
                i++;
            }
        } else {
            if (line[i] == '/' && line[i+1] == '*') {
                *in_block_comment = true;
                i += 2;
            } else if (line[i] == '/' && line[i+1] == '/') {
                break;
            } else {
                buffer[j++] = line[i++];
            }
        }
    }
    buffer[j] = '\0';
    return buffer;
}

void pass(bool n, FILE *file_in, FILE *file_out, Table *symtable) {
    uint64_t address = 0;
    IntermediateReps *ir = NULL;
    bool in_block_comment = false;
    if (n) {
        ir = malloc(sizeof(IntermediateReps));
    }
    fseek(file_in, 0, SEEK_SET);
    char *line = NULL;
    size_t len = 0;
    while (getline(&line, &len, file_in) != -1) {
        char *cleaned = remove_comments(line,&in_block_comment);
        char *trimmed = trim(cleaned);
        if (*trimmed == '\0')
            continue;
        char *p = strstr(trimmed, ":");
        if (p == NULL) { // instruction line
            if (n) {
                fprintf(stderr, "Entered instruction line: %s\n", trimmed);
                if (parse_line(trimmed, symtable, ir))
                    assemble_instructions(ir, address, file_out);
                else
                    fprintf(stderr, "Unrecognized line: %s\n", trimmed);
            }
            address += 4;
        } else if (!n) { // label line
            fprintf(stderr, "Entered label line: %s\n", trimmed);
            if (p[1] != '\0')
                fprintf(stderr, "Unrecognized label line: %s:\n", trimmed);
            *p = '\0';
            if (!isLabel(trimmed))
                fprintf(stderr, "Invalid label: %s\n", trimmed);

            put_entry(symtable, trimmed, address);
        }
    }
    free(line);
}
