#ifndef SYMBOLTABLE_H
#define SYMBOLTABLE_H

#include "arraylist.h"
#include "error.h"
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
    const char* identifier;
    type_t ret_type;
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
Symbol* SymbolTable_get(const SymbolTable* table, const char* identifier);

/**
 * @brief Get a symbol from its name, while respecting the scope
 * (local > param > global)
 *
 * @param table ProgramSymbolTable object
 * @param func FunctionSymbolTable object
 * @param identifier Name of the symbol to get
 * @return Symbol*
 */
Symbol* SymbolTable_resolve(const ProgramSymbolTable* table,
                            const FunctionSymbolTable* func,
                            const char* identifier);

/**
 * @brief Check if a called function was defined before use
 *
 * @param caller FunctionSymbolTable of the caller
 * @param callee FunctionSymbolTable of the function being called
 * @return true
 * @return false
 */
bool FunctionSymbolTable_is_defined_before_use(const FunctionSymbolTable* caller, const FunctionSymbolTable* callee);

/**
 * @brief Get a symbol from a node, while respecting the scope
 * (local > param > global)
 * If the symbol is not found, prints an error and return NULL
 *
 * @param table ProgramSymbolTable object
 * @param func FunctionSymbolTable object
 * @param node Identifier node
 * @return Symbol*
 */
Symbol* SymbolTable_resolve_from_node(const ProgramSymbolTable* table,
                                      const FunctionSymbolTable* func,
                                      const Node* node);

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
FunctionSymbolTable* FunctionSymbolTable_get_from_name(const ProgramSymbolTable* self,
                                                       const char* func_name);

/**
 * @brief Get the number of parameters of a function
 *
 * @param self
 * @return FunctionSymbolTable*
 */
int FunctionSymbolTable_get_param_count(const FunctionSymbolTable* self);

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
