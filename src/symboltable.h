#ifndef SYMBOLTABLE_H
#define SYMBOLTABLE_H

#include "symbol.h"
#include "arraylist.h"
#include "tree.h"

typedef struct SymbolTable {
    ArrayList symbols;
    size_t next_addr;
} SymbolTable;

typedef struct FunctionSymbolTable {
    SymbolTable parameters;
    SymbolTable locals;
} FunctionSymbolTable;

typedef struct ProgramSymbolTable {
    SymbolTable globals;
    ArrayList functions; // FunctionSymbolTable
} ProgramSymbolTable;

/**
 * @brief Initialize a symbol table
 * 
 * @param table 
 * @return int 
 */
int SymbolTable_init(SymbolTable* table);

/**
 * @brief Add a symbol to the table
 * 
 * @param table SymbolTable object
 * @param symbol Symbol to add
 * @return int 
 */
int SymbolTable_add(SymbolTable* table, Symbol symbol);

/**
 * @brief Get a symbol from the table
 * 
 * @param table SymbolTable object
 * @param identifier Identifier of the symbol to get
 * @return Symbol A pointer to the symbol, or NULL if the symbol is not found
 */
Symbol* SymbolTable_get(SymbolTable* table, char* identifier);

/**
 * @brief Create symbol table from a DeclVar tree
 * 
 * @param self SymbolTable object
 * @param tree Tree of DeclVars containing variables
 * @return int 
 */
int SymbolTable_from_DeclVar(SymbolTable* self, Tree tree);

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

#endif
