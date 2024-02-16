#ifndef SYBMOL_H
#define SYMBOL_H

#include "tree.h"
#include <stddef.h>

typedef struct Symbol {
    char* identifier;
    type_t type;
    size_t size;
    size_t addr;
} Symbol;

/**
 * @brief Initialize a symbol
 * 
 * @param self Symbol object to initialize
 * @param identifier Identifier of the symbol
 * @param type Type of the symbol
 * @param size Symbol's size
 * @return int 
 */
int Symbol_init(Symbol* self, char* identifier, type_t type, size_t size);

/**
 * @brief Compare two symbols by their identifier
 * 
 * @param a First symbol
 * @param b Second symbol
 * @return int
 * < 0 if a < b,
 * 0   if a == b,
 * > 0 if a > b
 */
int Symbol_cmp(const void* a, const void* b);

/**
 * @brief Print a symbol
 * 
 * @param symbol 
 */
void Symbol_print(const Symbol* symbol);

#endif
