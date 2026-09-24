#include <string.h>

#include "tokenizer.h"

size_t tokenize_line(const char *line, char *tokens[], const size_t max_tokens) {
    // copy the content in line to buffer
    static char buffer[1024];
    strncpy(buffer, line, sizeof(buffer) - 1);
    buffer[sizeof(buffer) - 1] = '\0'; // set the endpoint

    // initialize the tokenization
    char *saveptr = NULL;
    size_t count = 0;
    static const char *delimiters = " ,\t[";
    char *tok = strtok_r(buffer, delimiters, &saveptr);

    // deal with rest of buffer
    while (tok != NULL && count < max_tokens) {
        tokens[count++] = tok;
        tok = strtok_r(NULL, delimiters, &saveptr);
    }

    return count;
}
