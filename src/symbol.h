#ifndef SYMBOL_H
#define SYMBOL_H

#include "tree.h"
#include <stddef.h>
#include <stdbool.h>

#include "registers.h"

typedef enum SymbolType {
    SYMBOL_VALUE,
    SYMBOL_ARRAY,
    SYMBOL_POINTER_TO_ARRAY,
    SYMBOL_FUNCTION
} SymbolType;

typedef struct Symbol {
    char* identifier;
    type_t type;
    int type_size;
    int lineno;

    SymbolType symbol_type;
    union {
        struct {
            int length;
        } array;
    };

    int total_size;

    bool is_static;
    bool on_register;
    union { // First 6 parameters are stored in registers, rest are stored in stack
        Register reg;
        int addr;
    };
} Symbol;

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

/**
 * @brief Convert a symbol type to a string
 * 
 * @param type 
 * @return const char* 
 */
const char* SymbolType_to_str(SymbolType type);

#endif
