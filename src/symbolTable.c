#include "symbolTable.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "error.h"
#include "symbol.h"

#define BOLD "\x1b[1m"
#define UNDERLINE "\x1b[4m"
#define RESET "\x1b[0m"

/**
 * @brief Initialize a symbol table
 *
 * @param self SymbolTable object
 * @return ArrayListError
 */
static ArrayListError _ST_init(SymbolTable* self) {
    *self = (SymbolTable){
        ._next_addr_param = 16,
    };
    return ArrayList_init(&self->symbols, sizeof(Symbol), 30, Symbol_cmp);
}

static void _ST_free(SymbolTable* self) {
    ArrayList_free(&self->symbols);
    *self = (SymbolTable){0};
}

static void _FunctionST_free(FunctionST* self) {
    _ST_free(&self->parameters);
    _ST_free(&self->locals);
    *self = (FunctionST){0};
}

void ProgramST_free(ProgramST* self) {
    _ST_free(&self->globals);
    for (int i = 0; i < ArrayList_get_length(&self->functions); ++i) {
        FunctionST* function = ArrayList_get(&self->functions, i);
        _FunctionST_free(function);
    }
    ArrayList_free(&self->functions);
    *self = (ProgramST){0};
}

/**
 * @brief Initialize a FunctionST object
 *
 * @param self FunctionST object
 * @param identifier Function name, should be pre-allocated
 */
static void _FunctionST_init(FunctionST* self,
                             const char* identifier,
                             type_t ret_type) {
    *self = (FunctionST){
        .identifier = identifier,
        .ret_type = ret_type,
    };

    _ST_init(&self->parameters);
    self->parameters.type = SYMBOL_TABLE_PARAM;

    _ST_init(&self->locals);
    self->locals.type = SYMBOL_TABLE_LOCAL;
}

/**
 * @brief Add a symbol to the table
 *
 * @param self SymbolTable object
 * @param symbol Symbol to add
 * @return ErrorType ERR_SEM_REDECLARED_SYMBOL
 * if the symbol is already in the table
 */
ErrorType _ST_add(SymbolTable* self, Symbol symbol) {
    if (ArrayList_contains(&self->symbols, &symbol)) {
        CodeError_print(
            (CodeError){
                .err = ERR_SEM_REDECLARED_SYMBOL,
                .line = symbol.lineno,
                .column = symbol.column,
            },
            self->type == SYMBOL_TABLE_PARAM
                ? "redeclaration of parameter '%s'"
            : symbol.symbol_type == SYMBOL_FUNCTION
                ? "redefinition of function '%s'"
                : "redeclaration of '%s'",
            symbol.identifier);
        return ERR_SEM_REDECLARED_SYMBOL;
    }

    if (self->type == SYMBOL_TABLE_PARAM) {
        symbol.is_param = true;
    }

    /**
     * If the symbol is a parameter and there are less than 6 parameters
     * ours parameters will be on registers, having to store them later on
     * the callee's stack
     */
    if (self->type == SYMBOL_TABLE_PARAM) {
        if (ArrayList_get_length(&self->symbols) < 6) {
            // Those symbols will be copied on the callee's stack
            symbol.addr = -self->next_addr - symbol.total_size;
            self->next_addr += symbol.total_size;
        } else {
            symbol.addr = self->_next_addr_param;
            self->_next_addr_param += symbol.total_size;
        }
    } else if (self->type == SYMBOL_TABLE_GLOBAL) {
        symbol.addr = self->next_addr;
        self->next_addr += symbol.total_size;
    } else {
        symbol.addr = -self->next_addr - symbol.total_size;
        self->next_addr += symbol.total_size;
    }

    symbol.index = ArrayList_get_length(&self->symbols);
    ArrayList_sorted_insert(&self->symbols, &symbol);

    return ERR_NONE;
}

Symbol* ST_get(const SymbolTable* self, const char* identifier) {
    /* Returns a symbol associated to an identifier */

    const Symbol symbol = (Symbol){
        .identifier = identifier,
    };

    return ArrayList_search(&self->symbols, &symbol);
}

Symbol* ST_resolve(const ProgramST* table,
                   const FunctionST* func,
                   const char* identifier) {
    Symbol* symbol = NULL;

    if ((symbol = ST_get(&func->locals, identifier))) {
    } else if ((symbol = ST_get(&func->parameters, identifier))) {
    } else if ((symbol = ST_get(&table->globals, identifier)))
        ;

    return symbol;
}

bool FunctionST_is_defined_before_use(const FunctionST* caller,
                                      const FunctionST* callee) {
    //* The function array isn't sorted, and thus preserve insertion order
    //* Moreover, we only pass FunctionST through pointers
    return callee <= caller;
}

const Symbol* FunctionST_get_param(const FunctionST* self, int i) {
    for (int j = 0; j < ArrayList_get_length(&self->parameters.symbols); ++j) {
        const Symbol* symbol = ArrayList_get(&self->parameters.symbols, j);
        if (symbol->index == i) {
            return symbol;
        }
    }
    return NULL;
}

Symbol* ST_resolve_from_node(const ProgramST* table,
                             const FunctionST* func,
                             const Node* node) {
    Symbol* symbol = NULL;
    if ((symbol = ST_resolve(table, func, node->att.ident))) {
        return symbol;
    } else {
        // Variable not found
        CodeError_print(
            (CodeError){
                .err = ERR_UNDECLARED_SYMBOL,
                .line = node->lineno,
                .column = node->column,
            },
            "use of undeclared identifier '%s'",
            node->att.ident);
        exit(EXIT_CODE(ERR_UNDECLARED_SYMBOL));
    }
    return NULL;
}

/**
 * @brief Get size in bytes of a type
 *
 * @param type Type
 * @return size_t Type size in bytes
 */
static size_t _get_type_size(type_t type) {
    static const size_t sizes[] = {
        [type_byte] = sizeof(uint64_t),
        [type_num] = sizeof(uint64_t),
        [type_void] = 0,
    };
    assert(type == type_byte || type == type_num || type == type_void);

    return sizes[type];
}

/**
 * @brief Get the corresponding type from a string
 *
 * @param att Attribut object
 * @return type_t Type if found, -1 otherwise
 */
static type_t _get_type_from_string(const Attribut* att) {
    if (strcmp(att->key_word, "char") == 0) {
        return type_byte;
    } else if (strcmp(att->key_word, "int") == 0) {
        return type_num;
    }

    return -1;
}

/**
 * @brief Create a SymbolTable from a Type or DeclFonctArray
 * Iterate over all its siblings
 *
 * @param self SymbolTable to populate
 * @param tree Type/DeclFonctArray tree
 * @return ErrorType ERR_SEM_REDECLARED_SYMBOL
 * if a symbol was already in the table
 */
static ErrorType _ST_create_from_Type(SymbolTable* self, Tree tree) {
    assert(tree != NULL);
    assert(tree->label == Type || tree->label == DeclFonctArray);
    ErrorType err = 0;

    Tree typeNode = tree;
    FOREACH_SIBLING(typeNode) {
        Tree identNode = typeNode->firstChild;
        type_t type = _get_type_from_string(&typeNode->att);

        /**
         * We need to iterate a second time, for one-line declarations
         * (ex: int a, tab[10], b;)
         */
        FOREACH_SIBLING(identNode) {
            Symbol symbol;
            if (typeNode->label == DeclFonctArray) {
                symbol = (Symbol){
                    .identifier = identNode->att.ident,
                    .type = type,
                    .type_size = _get_type_size(type),
                    .symbol_type = SYMBOL_ARRAY,
                    .array.have_length = false,
                    .is_static = false,
                    .total_size = 8,
                    .lineno = identNode->lineno,
                    .column = identNode->column,
                };
            }

            else if (identNode->label == DeclArray) {
                int length = identNode->firstChild->att.num;

                if (length == 0) {
                    CodeError_print(
                        (CodeError){
                            .err = ADD_ERR(err, ERR_ARRAY_ZERO_SIZE),
                            .line = identNode->lineno,
                            .column = identNode->column,
                        },
                        "ISO C forbids zero-size array '%s'",
                        identNode->att.ident);
                }

                symbol = (Symbol){
                    .identifier = identNode->att.ident,
                    .is_static = self->type == SYMBOL_TABLE_GLOBAL,
                    .type = type,
                    .type_size = _get_type_size(type),
                    .symbol_type = SYMBOL_ARRAY,
                    .array.have_length = true,
                    .array.length = length,
                    .total_size = length * _get_type_size(type),
                    .lineno = identNode->lineno,
                    .column = identNode->column,
                };
            }

            else if (identNode->label == Ident) {
                symbol = (Symbol){
                    .identifier = identNode->att.ident,
                    .type = type,
                    .type_size = _get_type_size(type),
                    .is_static = self->type == SYMBOL_TABLE_GLOBAL,
                    .symbol_type = SYMBOL_VALUE,
                    .total_size = 1 * _get_type_size(type),
                    .lineno = identNode->lineno,
                    .column = identNode->column,
                };
            } else {
                assert(0 && "Unreachable");
            }
            err |= _ST_add(self, symbol);
        }
    }

    return err;
}

/**
 * @brief Create a symbol table from a DeclVars tree
 *
 * @param self SymbolTable object
 * @param offset Offset to start adding variables
 * (functions parameters could be stored on the stack if they are more than 6)
 * @param tree Tree of DeclVars containing variables
 * @return ErrorType ERR_SEM_REDECLARED_SYMBOL
 * if a symbol was already in the table
 */
static ErrorType ST_create_from_DeclVars(SymbolTable* self,
                                         int offset,
                                         Tree tree) {
    assert(tree->label == DeclVars);
    self->next_addr = offset;

    Node* node = tree->firstChild;
    // Empty DeclVars
    if (node == NULL) {
        return ERR_NONE;
    }

    return _ST_create_from_Type(self, node);
}

/**
 * @brief Add function parameters to the symbol table
 * Begins exploration from ListTypVar
 *
 * @param func FunctionST object to fill
 * @param tree Tree ListTypVar tree
 * @return ErrorType ERR_SEM_REDECLARED_SYMBOL
 * if a symbol is already in the table
 */
static ErrorType _FunctionST_create_from_ListTypVar(FunctionST* func,
                                                    Tree tree) {
    if (tree->label == Void) {
        return 0;
    }
    assert(tree->label == ListTypVar);
    // We go to the the first Type/DeclFontArray
    tree = tree->firstChild;

    return _ST_create_from_Type(&func->parameters, tree);
}

/**
 * @brief Check if a local variable is redeclared in the parameters list
 *
 * @param func FunctionST object to check
 * @return ErrorType ERR_SEM_REDECLARED_SYMBOL
 * if a symbol was already in the table
 */
static ErrorType _ST_check_redeclared_params_in_locals(FunctionST* func) {
    ErrorType err = ERR_NONE;

    for (int i = 0; i < ArrayList_get_length(&func->locals.symbols); ++i) {
        Symbol* local = ArrayList_get(&func->locals.symbols, i);
        Symbol* param = ST_get(&func->parameters, local->identifier);
        if (param) {
            CodeError_print(
                (CodeError){
                    .err = ADD_ERR(err, ERR_SEM_REDECLARED_SYMBOL),
                    .line = local->lineno,
                    .column = local->column,
                },
                "redeclaration of '%s' was already declared in parameters list",
                local->identifier);
        }
    }

    return err;
}

/**
 * @brief Create a FunctionST from a DeclFonct tree
 * Adds the function to the program's global symbol table
 * and creates a symbol table for the function's parameters and local variables
 *
 * @param prog SymbolTable of the program
 * @param func FunctionST of the function
 * @param func DeclFonct tree
 * @return ErrorType ERR_SEM_REDECLARED_SYMBOL
 * if a symbol is already in the table
 */
static ErrorType _ST_create_from_DeclFonct(ProgramST* prog,
                                           FunctionST* func,
                                           Tree tree) {
    if (!tree) {
        return ERR_NONE;
    }

    assert(tree->label == DeclFonct);
    ErrorType err = ERR_NONE;

    // Go to the EnTeteFonct tree
    Node* header = tree->firstChild;

    bool is_void = header->firstChild->type == type_void;

    Node* identNode = header->firstChild->nextSibling;

    type_t ret_type = is_void ? type_void
                              : _get_type_from_string(&header->firstChild->att);
    _FunctionST_init(func, identNode->att.ident, ret_type);

    // * Add the function to the program's global symbol table
    err |= _ST_add(
        &prog->globals,
        (Symbol){
            // function name
            .identifier = identNode->att.ident,
            .symbol_type = SYMBOL_FUNCTION,
            // return type
            .is_static = true,
            .type = func->ret_type,
            .type_size = _get_type_size(func->ret_type),
            .total_size = 0,
            .lineno = identNode->lineno,
            .column = identNode->column,
        });

    // * Add the function's parameters to the function's symbol table, if any
    Node* listParams = header->firstChild->nextSibling->nextSibling;
    err |= _FunctionST_create_from_ListTypVar(func, listParams);

    // * Add the function's local variables to the function's symbol table
    // Parameters could be stored on the stack
    int offset = func->parameters.next_addr;
    // DeclFonct->EnTeteFonct->Corps->DeclVars
    Node* listLocals = tree->firstChild->nextSibling->firstChild;
    err |= ST_create_from_DeclVars(&func->locals, offset, listLocals);

    err |= _ST_check_redeclared_params_in_locals(func);

    return err;
}

/**
 * @brief Starts tree exploration from a DeclFoncts
 * and creates a symbol table for each function
 *
 * @param self ProgramST object to fill
 * @param tree DeclFoncts tree to explore
 * @return ErrorType ERR_SEM_REDECLARED_SYMBOL
 * if a symbol is already in the table
 */
static ErrorType _ProgramST_from_DeclFoncts(ProgramST* self, Tree tree) {
    assert(tree->label == DeclFoncts);
    ErrorType err = ERR_NONE;

    Node* funcNode = tree->firstChild;
    FOREACH_SIBLING(funcNode) {
        FunctionST function;

        err |= _ST_create_from_DeclFonct(self, &function, funcNode);

        ArrayList_append(&self->functions, &function);
    }

    return err;
}

/**
 * @brief Add an hardcoded function to the progran
 * (putchar, getint, etc...)
 *
 * @param prog
 * @param ret A Symbol object representing the return value, with only
 * identifier and type defined
 * @param symbols Array of symbols to add as parameters.
 * Last element should be {0}
 * in order to stop the loop
 * (a NULL identifier is considered as the end of the array)
 * NULL if none (void parameter)
 */
static void ProgramST_add_default_function(ProgramST* prog,
                                           Symbol ret,
                                           Symbol symbols[]) {
    ret = (Symbol){
        .identifier = ret.identifier,
        .type = ret.type,
        .symbol_type = SYMBOL_FUNCTION,
        .is_static = true,
        .is_default_function = true,
        .type_size = _get_type_size(ret.type),
        .total_size = 1 * _get_type_size(ret.type),
    };
    _ST_add(&prog->globals, ret);

    FunctionST fun;
    _FunctionST_init(&fun, ret.identifier, ret.type);

    // If there are parameters, add them to the function's symbol table
    if (symbols != NULL) {
        // Loop until we find a NULL identifier (end of the array)
        for (const Symbol* param = symbols; param->identifier; ++param) {
            _ST_add(
                &fun.parameters,
                (Symbol){
                    .identifier = param->identifier,
                    .type = param->type,
                    .is_param = true,
                    .symbol_type = SYMBOL_VALUE,
                    .type_size = _get_type_size(param->type),
                    // A parameter never take an array by copy, only by pointer
                    .total_size = 1 * _get_type_size(param->type),
                });
        }
    }

    ArrayList_append(&prog->functions, &fun);
}

/**
 * @brief Add default functions to the global symbol table :
 * - putchar
 * - putint
 * - getchar
 * - getint
 *
 * @param globals SymbolTable object to fill
 */
static void _ST_add_default_functions(ProgramST* table) {
    ProgramST_add_default_function(
        table,
        (Symbol){
            .identifier = "putchar",
            .type = type_void,
        },
        (Symbol[]){
            (Symbol){
                .identifier = "character",
                .type = type_byte,
            },
            {0}});

    ProgramST_add_default_function(
        table,
        (Symbol){
            .identifier = "putint",
            .type = type_void,
        },
        (Symbol[]){
            (Symbol){
                .identifier = "number",
                .type = type_num,
            },
            {0}});

    ProgramST_add_default_function(
        table,
        (Symbol){
            .identifier = "getchar",
            .type = type_byte,
        },
        NULL);

    ProgramST_add_default_function(
        table,
        (Symbol){
            .identifier = "getint",
            .type = type_num,
        },
        NULL);
}

/**
 * @brief Create a symbol table from a Prog tree
 * - globals field contains:
 *  - global variables
 *  - functions prototypes
 *
 * functions is a list of FunctionST objects
 * Each FunctionST contains:
 * - parameters
 * - local variables
 *
 * @param self ProgramST object to fill
 * @param tree Prog tree
 * @return Error ERR_SEM_REDECLARED_SYMBOL if a symbol is already in the table
 */
ErrorType ProgramST_from_Prog(ProgramST* self, Tree tree) {
    ErrorType err = ERR_NONE;

    *self = (ProgramST){0};
    _ST_init(&self->globals);
    self->globals.type = SYMBOL_TABLE_GLOBAL;
    ArrayList_init(&self->functions, sizeof(FunctionST), 10, NULL);
    _ST_add_default_functions(self);

    // tree->firstChild is the a DeclVars tree of globals variables
    err |= ST_create_from_DeclVars(&self->globals, 0, tree->firstChild);

    // tree->firstChild->nextSibling is the first function to process
    err |= _ProgramST_from_DeclFoncts(self, tree->firstChild->nextSibling);

    return err;
}

FunctionST* FunctionST_get_from_name(const ProgramST* self,
                                     const char* func_name) {
    for (int i = 0; i < ArrayList_get_length(&self->functions); ++i) {
        FunctionST* function = ArrayList_get(&self->functions, i);
        if (!strcmp(function->identifier, func_name)) {
            return function;
        }
    }
    return NULL;
}

int FunctionST_get_param_count(const FunctionST* self) {
    return ArrayList_get_length(&self->parameters.symbols);
}

void ST_print(const SymbolTable* self) {
    for (int i = 0; i < ArrayList_get_length(&self->symbols); ++i) {
        Symbol* symbol = ArrayList_get(&self->symbols, i);
        Symbol_print(symbol);
    }
}

void FunctionST_print(const FunctionST* self) {
    printf(
        BOLD UNDERLINE "FunctionST of %s(...) -> %s:\n" RESET,
        self->identifier, Symbol_get_type_str(self->ret_type));
    printf(BOLD "Parameters:\n" RESET);
    ST_print(&self->parameters);
    printf(BOLD "Locals:\n" RESET);
    ST_print(&self->locals);
}

void ProgramST_print(const ProgramST* self) {
    printf(BOLD UNDERLINE "ProgramST:\n" RESET);
    printf(UNDERLINE "Globals:\n" RESET);
    ST_print(&self->globals);
    printf(UNDERLINE "Functions:\n\n" RESET);
    for (int i = 0; i < ArrayList_get_length(&self->functions); i++) {
        FunctionST* function = ArrayList_get(&self->functions, i);
        FunctionST_print(function);
        putchar('\n');
    }
}
