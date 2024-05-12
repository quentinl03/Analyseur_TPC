#include "symboltable.h"

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
static ArrayListError _SymbolTable_init(SymbolTable* self) {
    *self = (SymbolTable){
        ._next_addr_param = 16,
    };
    return ArrayList_init(&self->symbols, sizeof(Symbol), 30, Symbol_cmp);
}

/**
 * @brief Initialize a FunctionSymbolTable object
 *
 * @param self FunctionSymbolTable object
 * @param identifier Function name, should be pre-allocated
 */
static void _FunctionSymbolTable_init(FunctionSymbolTable* self, const char* identifier, type_t ret_type) {
    *self = (FunctionSymbolTable){
        .identifier = identifier,
        .ret_type = ret_type,
    };

    _SymbolTable_init(&self->parameters);
    self->parameters.type = SYMBOL_TABLE_PARAM;

    _SymbolTable_init(&self->locals);
    self->locals.type = SYMBOL_TABLE_LOCAL;
}

/**
 * @brief Add a symbol to the table
 *
 * @param self SymbolTable object
 * @param symbol Symbol to add
 * @return ErrorType ERR_SEM_REDECLARED_SYMBOL if the symbol is already in the table
 */
ErrorType _SymbolTable_add(SymbolTable* self, Symbol symbol) {
    if (ArrayList_contains(&self->symbols, &symbol)) {
        CodeError_print(
            (CodeError){
                .err = ERR_SEM_REDECLARED_SYMBOL,
                .line = symbol.lineno,
                .column = symbol.column,
            },
            self->type == SYMBOL_TABLE_PARAM        ? "redeclaration of parameter '%s'"
            : symbol.symbol_type == SYMBOL_FUNCTION ? "redefinition of function '%s'"
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
        }
        else {
            symbol.addr = self->_next_addr_param; 
            self->_next_addr_param += symbol.total_size;
        }
    }
    else if (self->type == SYMBOL_TABLE_GLOBAL) {
        symbol.addr = self->next_addr;
        self->next_addr += symbol.total_size;
    }
    else {
        symbol.addr = -self->next_addr - symbol.total_size;
        self->next_addr += symbol.total_size;
    }

    symbol.index = ArrayList_get_length(&self->symbols);
    ArrayList_sorted_insert(&self->symbols, &symbol);

    return ERR_NONE;
}

Symbol* SymbolTable_get(const SymbolTable* self, const char* identifier) {
    /* Returns a symbol associated to an identifier */

    const Symbol symbol = (Symbol){
        .identifier = identifier,
    };

    return ArrayList_search(&self->symbols, &symbol);
}

Symbol* SymbolTable_resolve(const ProgramSymbolTable* table,
                            const FunctionSymbolTable* func,
                            const char* identifier) {
    Symbol* symbol = NULL;

    if ((symbol = SymbolTable_get(&func->locals, identifier))) {
    } else if ((symbol = SymbolTable_get(&func->parameters, identifier))) {
    } else if ((symbol = SymbolTable_get(&table->globals, identifier)))
        ;

    return symbol;
}

bool FunctionSymbolTable_is_defined_before_use(const FunctionSymbolTable* caller, const FunctionSymbolTable* callee) {
    //* The function array isn't sorted, and thus preserve insertion order
    //* Moreover, we only pass FunctionSymbolTable through pointers
    return callee <= caller;
}

const Symbol* FunctionSymbolTable_get_param(const FunctionSymbolTable* self, int i) {
    for (int j = 0; j < ArrayList_get_length(&self->parameters.symbols); ++j) {
        const Symbol* symbol = ArrayList_get(&self->parameters.symbols, j);
        if (symbol->index == i) {
            return symbol;
        }
    }
    return NULL;
}

Symbol* SymbolTable_resolve_from_node(const ProgramSymbolTable* table,
                                      const FunctionSymbolTable* func,
                                      const Node* node) {
    Symbol* symbol = NULL;
    if ((symbol = SymbolTable_resolve(table, func, node->att.ident))) {
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
 * @return ErrorType ERR_SEM_REDECLARED_SYMBOL if a symbol was already in the table
 */
static ErrorType _SymbolTable_create_from_Type(SymbolTable* self, Tree tree) {
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

                    // TODO: Is this ok? An array is decayed to a pointer in C (8 bytes on x86_64 systems)
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
                        identNode->att.ident
                    );
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
            }
            else {
              assert(0 && "Unreachable");
            }
            err |= _SymbolTable_add(self, symbol);
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
 * @return ErrorType ERR_SEM_REDECLARED_SYMBOL if a symbol was already in the table
 */
static ErrorType SymbolTable_create_from_DeclVars(SymbolTable* self, int offset, Tree tree) {
    assert(tree->label == DeclVars);
    self->next_addr = offset;

    Node* node = tree->firstChild;
    // Empty DeclVars
    if (node == NULL) {
        return ERR_NONE;
    }

    return _SymbolTable_create_from_Type(self, node);
}

/**
 * @brief Add function parameters to the symbol table
 * Begins exploration from ListTypVar
 *
 * @param func FunctionSymbolTable object to fill
 * @param tree Tree ListTypVar tree
 * @return ErrorType ERR_SEM_REDECLARED_SYMBOL if a symbol is already in the table
 */
static ErrorType _FunctionSymbolTable_create_from_ListTypVar(FunctionSymbolTable* func, Tree tree) {
    if (tree->label == Void) {
        return 0;
    }
    assert(tree->label == ListTypVar);
    // We go to the the first Type/DeclFontArray
    tree = tree->firstChild;

    return _SymbolTable_create_from_Type(&func->parameters, tree);
}

/**
 * @brief Check if a local variable is redeclared in the parameters list
 *
 * @param func FunctionSymbolTable object to check
 * @return ErrorType ERR_SEM_REDECLARED_SYMBOL if a symbol was already in the table
 */
static ErrorType _SymbolTable_check_redeclared_params_in_locals(FunctionSymbolTable* func) {
    ErrorType err = ERR_NONE;

    for (int i = 0; i < ArrayList_get_length(&func->locals.symbols); ++i) {
        Symbol* local = ArrayList_get(&func->locals.symbols, i);
        Symbol* param = SymbolTable_get(&func->parameters, local->identifier);
        if (param) {
            CodeError_print(
                (CodeError){
                    .err = ADD_ERR(err, ERR_SEM_REDECLARED_SYMBOL),
                    .line = local->lineno,
                    .column = local->column,
                },
                "redeclaration of variable '%s' already declared in parameters list",
                local->identifier);
        }
    }

    return err;
}

/**
 * @brief Create a FunctionSymbolTable from a DeclFonct tree
 * Adds the function to the program's global symbol table
 * and creates a symbol table for the function's parameters and local variables
 *
 * @param prog SymbolTable of the program
 * @param func FunctionSymbolTable of the function
 * @param func DeclFonct tree
 * @return ErrorType ERR_SEM_REDECLARED_SYMBOL if a symbol is already in the table
 */
static ErrorType _SymbolTable_create_from_DeclFonct(ProgramSymbolTable* prog, FunctionSymbolTable* func, Tree tree) {
    if (!tree) {
        return ERR_NONE;
    }

    assert(tree->label == DeclFonct);
    ErrorType err = ERR_NONE;

    // Go to the EnTeteFonct tree
    Node* header = tree->firstChild;

    bool is_void = header->firstChild->type == type_void;

    Node* identNode = header->firstChild->nextSibling;

    type_t ret_type = is_void ? type_void : _get_type_from_string(&header->firstChild->att);
    _FunctionSymbolTable_init(func, identNode->att.ident, ret_type);

    // * Add the function to the program's global symbol table
    err |= _SymbolTable_add(
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
    err |= _FunctionSymbolTable_create_from_ListTypVar(func, listParams);

    // * Add the function's local variables to the function's symbol table
    int offset = func->parameters.next_addr;  // Parameters could be stored on the stack
    // DeclFonct->EnTeteFonct->Corps->DeclVars
    Node* listLocals = tree->firstChild->nextSibling->firstChild;
    err |= SymbolTable_create_from_DeclVars(&func->locals, offset, listLocals);

    err |= _SymbolTable_check_redeclared_params_in_locals(func);

    return err;
}

/**
 * @brief Starts tree exploration from a DeclFoncts
 * and creates a symbol table for each function
 *
 * @param self ProgramSymbolTable object to fill
 * @param tree DeclFoncts tree to explore
 * @return ErrorType ERR_SEM_REDECLARED_SYMBOL if a symbol is already in the table
 */
static ErrorType _ProgramSymbolTable_from_DeclFoncts(ProgramSymbolTable* self, Tree tree) {
    assert(tree->label == DeclFoncts);
    ErrorType err = ERR_NONE;

    Node* funcNode = tree->firstChild;
    FOREACH_SIBLING(funcNode) {
        FunctionSymbolTable function;

        err |= _SymbolTable_create_from_DeclFonct(self, &function, funcNode);

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
 * @param symbols Array of symbols to add as parameters. Last element should be {0}
 * in order to stop the loop (a NULL identifier is considered as the end of the array)
 * NULL if none (void parameter)
 */
static void ProgramSymbolTable_add_default_function(ProgramSymbolTable* prog,
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
    _SymbolTable_add(&prog->globals, ret);

    FunctionSymbolTable fun;
    _FunctionSymbolTable_init(&fun, ret.identifier, ret.type);

    // If there are parameters, add them to the function's symbol table
    if (symbols != NULL) {
        // Loop until we find a NULL identifier (end of the array)
        for (const Symbol* param = symbols; param->identifier; ++param) {
            _SymbolTable_add(
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
static void _SymbolTable_add_default_functions(ProgramSymbolTable* table) {
    ProgramSymbolTable_add_default_function(
        table,
        (Symbol){
            .identifier = "putchar",
            .type = type_num,
        },
        (Symbol[]){
            (Symbol){
                .identifier = "character",
                .type = type_byte,
            },
            {0}});

    ProgramSymbolTable_add_default_function(
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

    ProgramSymbolTable_add_default_function(
        table,
        (Symbol){
            .identifier = "getchar",
            .type = type_byte,
        },
        NULL);

    ProgramSymbolTable_add_default_function(
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
 * functions is a list of FunctionSymbolTable objects
 * Each FunctionSymbolTable contains:
 * - parameters
 * - local variables
 *
 * @param self ProgramSymbolTable object to fill
 * @param tree Prog tree
 * @return Error ERR_SEM_REDECLARED_SYMBOL if a symbol is already in the table
 */
ErrorType ProgramSymbolTable_from_Prog(ProgramSymbolTable* self, Tree tree) {
    ErrorType err = ERR_NONE;

    *self = (ProgramSymbolTable){0};
    _SymbolTable_init(&self->globals);
    self->globals.type = SYMBOL_TABLE_GLOBAL;
    ArrayList_init(&self->functions, sizeof(FunctionSymbolTable), 10, NULL);
    _SymbolTable_add_default_functions(self);

    // tree->firstChild is the a DeclVars tree of globals variables
    err |= SymbolTable_create_from_DeclVars(&self->globals, 0, tree->firstChild);

    // tree->firstChild->nextSibling is the first function to process
    err |= _ProgramSymbolTable_from_DeclFoncts(self, tree->firstChild->nextSibling);

    return err;
}

FunctionSymbolTable* FunctionSymbolTable_get_from_name(const ProgramSymbolTable* self,
                                                       const char* func_name) {
    for (int i = 0; i < ArrayList_get_length(&self->functions); ++i) {
        FunctionSymbolTable* function = ArrayList_get(&self->functions, i);
        if (!strcmp(function->identifier, func_name)) {
            return function;
        }
    }
    return NULL;
}

int FunctionSymbolTable_get_param_count(const FunctionSymbolTable* self) {
    return ArrayList_get_length(&self->parameters.symbols);
}

void SymbolTable_print(const SymbolTable* self) {
    for (int i = 0; i < ArrayList_get_length(&self->symbols); ++i) {
        Symbol* symbol = ArrayList_get(&self->symbols, i);
        Symbol_print(symbol);
    }
}

void FunctionSymbolTable_print(const FunctionSymbolTable* self) {
    printf(
        BOLD UNDERLINE "FunctionSymbolTable of %s(...) -> %s:\n" RESET,
        self->identifier, Symbol_get_type_str(self->ret_type));
    printf(BOLD "Parameters:\n" RESET);
    SymbolTable_print(&self->parameters);
    printf(BOLD "Locals:\n" RESET);
    SymbolTable_print(&self->locals);
}

void ProgramSymbolTable_print(const ProgramSymbolTable* self) {
    printf(BOLD UNDERLINE "ProgramSymbolTable:\n" RESET);
    printf(UNDERLINE "Globals:\n" RESET);
    SymbolTable_print(&self->globals);
    printf(UNDERLINE "Functions:\n\n" RESET);
    for (int i = 0; i < ArrayList_get_length(&self->functions); i++) {
        FunctionSymbolTable* function = ArrayList_get(&self->functions, i);
        FunctionSymbolTable_print(function);
        putchar('\n');
    }
}
