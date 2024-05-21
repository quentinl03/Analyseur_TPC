#ifndef SymbolTable_H
#define SymbolTable_H

#include "arraylist.h"
#include "error.h"
#include "symbol.h"
#include "tree.h"

typedef enum STType {
    SYMBOL_TABLE_GLOBAL = 1,
    SYMBOL_TABLE_LOCAL,
    SYMBOL_TABLE_PARAM
} STType;

typedef struct SymbolTable {
    ArrayList symbols;  // [Symbol]
    size_t next_addr;
    STType type;
    int _next_addr_param; /*<
        Used with symbol table of type SYMBOL_TABLE_PARAM.
        Next address for 6+ parameters.
    */
} SymbolTable;

typedef struct FunctionST {
    const char* identifier;
    type_t ret_type;
    SymbolTable parameters;
    SymbolTable locals;
} FunctionST;

typedef struct ProgramST {
    SymbolTable globals;
    ArrayList functions;  // [FunctionST]
} ProgramST;

/**
 * @brief Get a symbol from the table
 *
 * @param table SymbolTable object
 * @param identifier Identifier of the symbol to get
 * @return Symbol A pointer to the symbol, or NULL if the symbol is not found
 */
Symbol* ST_get(const SymbolTable* table, const char* identifier);

/**
 * @brief Get a symbol from its name, while respecting the scope
 * (local > param > global)
 *
 * @param table ProgramST object
 * @param func FunctionST object
 * @param identifier Name of the symbol to get
 * @return Symbol*
 */
Symbol* ST_resolve(const ProgramST* table,
                   const FunctionST* func,
                   const char* identifier);

/**
 * @brief Check if a called function was defined before use
 *
 * @param caller FunctionST of the caller
 * @param callee FunctionST of the function being called
 * @return true
 * @return false
 */
bool FunctionST_is_defined_before_use(const FunctionST* caller,
                                      const FunctionST* callee);

/**
 * @brief Get the i-th parameter of a function
 *
 * @param self
 * @param i index of the parameter
 * @return const Symbol*
 */
const Symbol* FunctionST_get_param(const FunctionST* self, int i);

/**
 * @brief Get a symbol from a node, while respecting the scope
 * (local > param > global)
 * If the symbol is not found, prints an error and return NULL
 *
 * @param table ProgramST object
 * @param func FunctionST object
 * @param node Identifier node
 * @return Symbol*
 */
Symbol* ST_resolve_from_node(const ProgramST* table,
                             const FunctionST* func,
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
ErrorType ProgramST_from_Prog(ProgramST* self, Tree tree);

/**
 * @brief Get a FunctionST from the function name. If the function is not found,
 * return NULL
 *
 * @param self
 * @param func_name
 * @return FunctionST*
 */
FunctionST* FunctionST_get_from_name(const ProgramST* self,
                                     const char* func_name);

/**
 * @brief Get the number of parameters of a function
 *
 * @param self
 * @return FunctionST*
 */
int FunctionST_get_param_count(const FunctionST* self);

/**
 * @brief Print the symbol table
 *
 * @param self SymbolTable object
 */
void ST_print(const SymbolTable* self);

/**
 * @brief Print the function symbol table
 *
 * @param self
 */
void FunctionST_print(const FunctionST* self);

/**
 * @brief Print the program symbol table
 *
 * @param self
 */
void ProgramST_print(const ProgramST* self);

/**
 * @brief Free the memory allocated by a SymbolTable object
 *
 * @param self
 */
void ProgramST_free(ProgramST* self);

#endif
