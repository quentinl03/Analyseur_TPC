#ifndef SYMBOLTABLE_H
#define SYMBOLTABLE_H

#include "arraylist.h"
#include "symbol.h"
#include "tree.h"
#include "error.h"

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
 * @brief Get a symbol from the table
 *
 * @param table SymbolTable object
 * @param identifier Identifier of the symbol to get
 * @return Symbol A pointer to the symbol, or NULL if the symbol is not found
 */
Symbol* SymbolTable_get(const SymbolTable* table, char* identifier);

/**
 * @brief Create symbol table of program's global variables
 * and each function's parameters and local variables
 *
 * @param self
 * @param tree
 * @return ErrorType ERR_NONE if no error, otherwise the error
 * 
 */
ErrorType ProgramSymbolTable_from_Prog(ProgramSymbolTable* self, Tree tree);

/**
 * @brief Get a FunctionSymbolTable from the function name. If the function is not found, return NULL
 *
 * @param self
 * @param func_name
 * @return FunctionSymbolTable*
 */
FunctionSymbolTable* SymbolTable_get_from_func_name(const ProgramSymbolTable* self,
                                                    const char* func_name);

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
