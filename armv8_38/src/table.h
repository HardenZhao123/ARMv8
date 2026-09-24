#ifndef TABLE_H
#define TABLE_H

#include <stdint.h>

// Linked List Entry
typedef struct Entry {
  //key: label
  char *key;
  //value: address of the label
  uint64_t value;
  //next: next entry in linked list
  struct Entry *next;
} Entry;

// Head of Linked List
typedef struct {
    Entry *head;
    Entry *tail;
} Table;

/**
 * @brief It initialises and returns a new symbol table.
 * The symbol table maps labels (as keys) to their corresponding addresses (as values).
 *
 * @return A pointer to an empty symbol table with label-address mappings.
 */
Table *initialize_table(void);

/**
 * Put an entry into the table
 * @param table The table to put the entry into
 * @param key The key to put the entry into
 * @param value The value to put the entry into
 */
void put_entry(Table *table, const char *key, uint64_t value);

/**
 * Get the value of an entry in the table
 * @param table The table to search
 * @param key The key to search for
 * @return The value of the entry, or -1 if the key is not found
 */
uint64_t get_entry(const Table *table, const char *key);

/**
 * @brief Frees all memory associated with the symbol table.
 * @param table A pointer to the symbol table to free
 *              It includes all keys, entries, and the symbol table itself.
 */
void free_table(Table *table);

#endif
