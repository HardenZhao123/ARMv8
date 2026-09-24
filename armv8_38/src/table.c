#define _POSIX_C_SOURCE 200809L

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "table.h"

#define NOT_FOUND UINT64_MAX

// Initialize a table, don't forget to free it
Table *initialize_table(void) {
    Table *table = malloc(sizeof(Table));
    if (table == NULL) {
        fprintf(stderr, "Failed to allocate memory for table\n");
        return NULL;
    }
    table->head = NULL;
    table->tail = NULL;
    return table;
}

void put_entry(Table *table, const char *key, uint64_t value) {
    Entry *entry = malloc(sizeof(Entry));
    if (entry == NULL) {
        fprintf(stderr, "Failed to allocate memory for entry\n");
        return;
    }
    entry->key = strdup(key);
    entry->value = value;
    entry->next = NULL;
    if (table->head == NULL) {
        table->head = entry;
        table->tail = entry;
    } else {
        table->tail->next = entry;
        table->tail = entry;
    }
}

uint64_t get_entry(const Table *table, const char *key) {
    Entry *current = table->head;
    while (current != NULL) {
        uint64_t out = current->value;
        if (strcmp(current->key, key) == 0) {
            return out;
        }
        current = current->next;
    }
    return NOT_FOUND;
}

void free_table(Table *table) {
    Entry *current = table->head;
    while (current != NULL) {
        Entry *next = current->next;
        free(current->key);
        free(current);
        current = next;
    }
    free(table);
}
