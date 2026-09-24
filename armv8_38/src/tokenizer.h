#ifndef TOKENIZER_H
#define TOKENIZER_H

#include <stdio.h>

/**
 *
 * @param line The input string line to tokenise.
 * @param tokens An array to store the pointers of each token.
 * @param max_tokens Maximum number of tokens can be stored.
 * @return The number of tokens found in that string line.
 */
extern size_t tokenize_line(const char *line, char *tokens[], size_t max_tokens);

#endif
