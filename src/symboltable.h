#ifndef SYMBOLTABLE_H
#define SYMBOLTABLE_H

#include "arraylist.h"
#include "symbol.h"
#include "tree.h"

typedef enum SymbolTableType {
    SYMBOL_TABLE_GLOBAL = 1,
    SYMBOL_TABLE_LOCAL,
    SYMBOL_TABLE_PARAM
} SymbolTableType;

typedef struct SymbolTable {
    ArrayList symbols;  // [Symbol]
    SymbolTableType type;
    size_t next_addr;
} SymbolTable;

typedef struct FunctionSymbolTable {
    char* identifier;
    SymbolTable parameters;
    SymbolTable locals;
} FunctionSymbolTable;

typedef struct ProgramSymbolTable {
    SymbolTable globals;
    ArrayList functions;  // [FunctionSymbolTable]
} ProgramSymbolTable;

/**
 * @brief Add a symbol to the table
 *
 * @param table SymbolTable object
 * @param symbol Symbol to add
 * @return int
 */
int _SymbolTable_add(SymbolTable* table, Symbol symbol);

/**
 * @brief Get a symbol from the table
 *
 * @param table SymbolTable object
 * @param identifier Identifier of the symbol to get
 * @return Symbol A pointer to the symbol, or NULL if the symbol is not found
 */
Symbol* SymbolTable_get(SymbolTable* table, char* identifier);

/**
 * @brief Create symbol table of program's global variables
 * and each function's parameters and local variables
 *
 * @param self
 * @param tree
 * @return int
 */
int ProgramSymbolTable_from_Prog(ProgramSymbolTable* self, Tree tree);

/**
 * @brief Print the symbol table
 *
 * @param self SymbolTable object
 */
void SymbolTable_print(const SymbolTable* self);

/**
 * @brief Print the function symbol table
 *
 * @param self
 */
void FunctionSymbolTable_print(const FunctionSymbolTable* self);

/**
 * @brief Print the program symbol table
 *
 * @param self
 */
void ProgramSymbolTable_print(const ProgramSymbolTable* self);

#endif
