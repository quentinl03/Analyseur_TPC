#include "symboltable.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "symbol.h"
#include "error.h"

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
    *self = (SymbolTable){0};
    return ArrayList_init(&self->symbols, sizeof(Symbol), 30, Symbol_cmp);
}

/**
 * @brief Initialize a FunctionSymbolTable object
 *
 * @param self FunctionSymbolTable object
 * @param identifier Function name, should be pre-allocated
 */
static void _FunctionSymbolTable_init(FunctionSymbolTable* self, char* identifier) {
    *self = (FunctionSymbolTable){
        .identifier = identifier,
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
            (CodeError) {
                .err = ERR_SEM_REDECLARED_SYMBOL,
                .line = symbol.lineno,
                .column = 0,
            },
            self->type == SYMBOL_TABLE_PARAM ? "redeclaration of parameter '%s'"
                : symbol.symbol_type == SYMBOL_FUNCTION ? "redefinition of function '%s'"
                : "redeclaration of '%s'",
            symbol.identifier
        );
        return ERR_SEM_REDECLARED_SYMBOL;
    }

    /**
     * If the symbol is a parameter and there are less than 6 parameters
     * on this function symbol table, we put parameters on registers
     */
    if (self->type == SYMBOL_TABLE_PARAM && ArrayList_get_length(&self->symbols) < 6) {
        symbol.on_register = true;
        symbol.reg = Register_param_to_reg(ArrayList_get_length(&self->symbols));
    }
    /**
     * Else, we used all registers, and parameters are stored on the stack.
     */
    else {
        symbol.on_register = false;
        symbol.addr = self->next_addr;
        self->next_addr += symbol.total_size;
    }

    assert(ArrayList_sorted_insert(&self->symbols, &symbol) != ARRAYLIST_ERR_ALLOC);

    return ERR_NONE;
}

Symbol* SymbolTable_get(const SymbolTable* self, char* identifier) {
    /* Returns a symbol associated to an identifier */

    Symbol symbol = (Symbol){
        .identifier = identifier,
    };

    return ArrayList_search(&self->symbols, &symbol);
}

/**
 * @brief Get size in bytes of a type
 *
 * @param type Type
 * @return size_t Type size in bytes
 */
static size_t _get_type_size(type_t type) {
    static const size_t sizes[] = {
        [type_byte] = sizeof(char),
        [type_num] = sizeof(int),
        [type_void] = 0,
    };
    assert(type >= 0 && type < 5);

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

    type_t type = _get_type_from_string(&tree->att);

    Tree typeNode = tree;
    FOREACH_SIBLING(typeNode) {
        Tree identNode = typeNode->firstChild;

        /**
         * We need to iterate a second time, for one-line declarations
         * (ex: int a, tab[10], b;)
         */
        FOREACH_SIBLING(identNode) {
            Symbol symbol;
            if (typeNode->label == DeclFonctArray) {
                symbol = (Symbol) {
                    .identifier = identNode->att.ident,
                    .type = type,
                    .type_size = _get_type_size(type),
                    .symbol_type = SYMBOL_POINTER_TO_ARRAY,
                    .is_static = false,

                    // TODO: Is this ok? An array is decayed to a pointer in C (8 bytes on x86_64 systems)
                    .total_size = 8,
                    .lineno = identNode->lineno,
                };
            }

            else if (identNode->label == DeclArray) {
                symbol = (Symbol) {
                    .identifier = identNode->att.ident,
                    .is_static = self->type == SYMBOL_TABLE_GLOBAL,
                    .type = type,
                    .type_size = _get_type_size(type),
                    .symbol_type = SYMBOL_ARRAY,
                    .array.length = identNode->firstChild->att.num,
                    .total_size = identNode->firstChild->att.num * _get_type_size(type),
                    .lineno = identNode->lineno,
                };
            }

            else if (identNode->label == Ident) {
                symbol = (Symbol) {
                    .identifier = identNode->att.ident,
                    .type = type,
                    .type_size = _get_type_size(type),
                    .is_static = self->type == SYMBOL_TABLE_GLOBAL,
                    .symbol_type = SYMBOL_VALUE,
                    .total_size = 1 * _get_type_size(type),
                    .lineno = identNode->lineno,
                };
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
                (CodeError) {
                    .err = ERR_SEM_REDECLARED_SYMBOL,
                    .line = local->lineno,
                    .column = 0,
                },
                "redeclaration of variable '%s' already declared in parameters list",
                local->identifier
            );
            err |= ERR_SEM_REDECLARED_SYMBOL;
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

    ErrorType err = ERR_NONE;

    // Go to the EnTeteFonct tree
    Node* header = tree->firstChild;

    assert(tree->label == DeclFonct);
    bool is_void = header->firstChild->type == type_void;

    Node* identNode = header->firstChild->nextSibling;
    _FunctionSymbolTable_init(func, identNode->att.ident);

    type_t return_type = is_void ? type_void : _get_type_from_string(&header->firstChild->att);

    // * Add the function to the program's global symbol table
    err |= _SymbolTable_add(
        &prog->globals,
        (Symbol) {
            // function name
            .identifier = identNode->att.ident,
            .symbol_type = SYMBOL_FUNCTION,
            // return type
            .is_static = true,
            .type = return_type,
            .type_size = _get_type_size(return_type),
            .total_size = 0,
            .lineno = identNode->lineno,
        }
    );

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
    ArrayList_init(&self->functions, sizeof(FunctionSymbolTable), 10, NULL);

    // tree->firstChild is the a DeclVars tree of globals variables
    self->globals.type = SYMBOL_TABLE_GLOBAL;
    err |= SymbolTable_create_from_DeclVars(&self->globals, 0, tree->firstChild);

    // tree->firstChild->nextSibling is the first function to process
    err |= _ProgramSymbolTable_from_DeclFoncts(self, tree->firstChild->nextSibling);

    return err;
}

FunctionSymbolTable* SymbolTable_get_from_func_name(const ProgramSymbolTable* self,
                                                    const char* func_name) {
    for (int i = 0; i < ArrayList_get_length(&self->functions); ++i) {
        FunctionSymbolTable* function = ArrayList_get(&self->functions, i);
        if (!strcmp(function->identifier, func_name)) {
            return function;
        }
    }
    return NULL;
}

void SymbolTable_print(const SymbolTable* self) {
    for (int i = 0; i < ArrayList_get_length(&self->symbols); ++i) {
        Symbol* symbol = ArrayList_get(&self->symbols, i);
        Symbol_print(symbol);
    }
}

void FunctionSymbolTable_print(const FunctionSymbolTable* self) {
    printf(BOLD UNDERLINE "FunctionSymbolTable of %s():\n" RESET, self->identifier);
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
