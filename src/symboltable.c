#include "symboltable.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

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
 * @return int
 *  - 1 if the symbol is already in the table
 *  - 0 if the symbol was added
 *  - -1 if an error occured
 */
int _SymbolTable_add(SymbolTable* self, Symbol symbol) {
    if (ArrayList_contains(&self->symbols, &symbol))
        return 1;

    ArrayListError err = 0;

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

    if ((err = ArrayList_sorted_insert(&self->symbols, &symbol)) < -1) {
        return -1;
    }

    return 0;
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
 * @return int
 */
static int _SymbolTable_create_from_Type(SymbolTable* self, Tree tree) {
    assert(tree != NULL);
    assert(tree->label == Type || tree->label == DeclFonctArray);

    type_t type = _get_type_from_string(&tree->att);

    Tree typeNode = tree;
    FOREACH_SIBLING(typeNode) {
        Tree identNode = typeNode->firstChild;

        /**
         * We need to iterate a second time, for one-line declarations
         * (ex: int a, tab[10], b;)
         */
        FOREACH_SIBLING(identNode) {
            if (typeNode->label == DeclFonctArray) {
                _SymbolTable_add(
                    self,
                    (Symbol){
                        .identifier = identNode->att.ident,
                        .type = type,
                        .type_size = _get_type_size(type),
                        .symbol_type = SYMBOL_POINTER_TO_ARRAY,
                        .is_static = false,

                        // TODO: Is this ok?
                        .total_size = 8,  // An array is decayed to a pointer in C (8 bytes on x86_64 systems)
                        .lineno = identNode->lineno,
                    });
            }

            else if (identNode->label == DeclArray) {
                _SymbolTable_add(
                    self,
                    (Symbol){
                        .identifier = identNode->att.ident,
                        .is_static = self->type == SYMBOL_TABLE_GLOBAL,
                        .type = type,
                        .type_size = _get_type_size(type),
                        .symbol_type = SYMBOL_ARRAY,
                        .array.length = identNode->firstChild->att.num,
                        .total_size = identNode->firstChild->att.num * _get_type_size(type),
                        .lineno = identNode->lineno,
                    });
            }

            else if (identNode->label == Ident) {
                _SymbolTable_add(
                    self,
                    (Symbol){
                        .identifier = identNode->att.ident,
                        .type = type,
                        .type_size = _get_type_size(type),
                        .is_static = self->type == SYMBOL_TABLE_GLOBAL,
                        .symbol_type = SYMBOL_VALUE,
                        .total_size = 1 * _get_type_size(type),
                        .lineno = identNode->lineno,
                    });
            }
        }
    }

    return 1;
}

/**
 * @brief Create a symbol table from a DeclVars tree
 *
 * @param self SymbolTable object
 * @param offset Offset to start adding variables
 * (functions parameters could be stored on the stack if they are more than 6)
 * @param tree Tree of DeclVars containing variables
 * @return int
 */
static int SymbolTable_create_from_DeclVars(SymbolTable* self, int offset, Tree tree) {
    assert(tree->label == DeclVars);
    self->next_addr = offset;

    Node* node = tree->firstChild;
    // Empty DeclVars
    if (node == NULL) {
        return 0;
    }

    return _SymbolTable_create_from_Type(self, node);
}

/**
 * @brief Add function parameters to the symbol table
 * Begins exploration from ListTypVar
 *
 * @param func FunctionSymbolTable object to fill
 * @param tree Tree ListTypVar tree
 * @return int
 */
static int _FunctionSymbolTable_create_from_ListTypVar(FunctionSymbolTable* func, Tree tree) {
    if (tree->label == Void) {
        return 0;
    }
    assert(tree->label == ListTypVar);
    // We go to the the first Type/DeclFontArray
    tree = tree->firstChild;

    _SymbolTable_create_from_Type(&func->parameters, tree);

    return 0;
}

/**
 * @brief Create a FunctionSymbolTable from a DeclFonct tree
 * Adds the function to the program's global symbol table
 * and creates a symbol table for the function's parameters and local variables
 *
 * @param prog SymbolTable of the program
 * @param func FunctionSymbolTable of the function
 * @param func DeclFonct tree
 * @return int
 */
static int _SymbolTable_create_from_DeclFonct(ProgramSymbolTable* prog, FunctionSymbolTable* func, Tree tree) {
    if (!tree) {
        return -1;
    }

    // Go to the EnTeteFonct tree
    Node* header = tree->firstChild;

    assert(tree->label == DeclFonct);
    bool is_void = header->firstChild->type == type_void;

    Node* identNode = header->firstChild->nextSibling;
    _FunctionSymbolTable_init(func, identNode->att.ident);

    type_t return_type = is_void ? type_void : _get_type_from_string(&header->firstChild->att);

    // * Add the function to the program's global symbol table
    _SymbolTable_add(
        &prog->globals,
        (Symbol){
            // function name
            .identifier = identNode->att.ident,
            .symbol_type = SYMBOL_FUNCTION,
            // return type
            .is_static = true,
            .type = return_type,
            .type_size = _get_type_size(return_type),
            .total_size = 0,
            .lineno = identNode->lineno,
        });

    // * Add the function's parameters to the function's symbol table, if any
    Node* listParams = header->firstChild->nextSibling->nextSibling;
    _FunctionSymbolTable_create_from_ListTypVar(func, listParams);

    // * Add the function's local variables to the function's symbol table
    int offset = func->parameters.next_addr;  // Parameters could be stored on the stack
    // DeclFonct->EnTeteFonct->Corps->DeclVars
    Node* listLocals = tree->firstChild->nextSibling->firstChild;
    SymbolTable_create_from_DeclVars(&func->locals, offset, listLocals);

    return 0;
}

/**
 * @brief Starts tree exploration from a DeclFoncts
 * and creates a symbol table for each function
 *
 * @param self ProgramSymbolTable object to fill
 * @param tree DeclFoncts tree to explore
 * @return int
 */
static int _ProgramSymbolTable_from_DeclFoncts(ProgramSymbolTable* self, Tree tree) {
    assert(tree->label == DeclFoncts);

    Node* funcNode = tree->firstChild;
    FOREACH_SIBLING(funcNode) {
        FunctionSymbolTable function;

        _SymbolTable_create_from_DeclFonct(self, &function, funcNode);

        ArrayList_append(&self->functions, &function);
    }

    return 1;
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
 * @return int
 */
int ProgramSymbolTable_from_Prog(ProgramSymbolTable* self, Tree tree) {
    *self = (ProgramSymbolTable){0};
    _SymbolTable_init(&self->globals);
    ArrayList_init(&self->functions, sizeof(FunctionSymbolTable), 10, NULL);

    // tree->firstChild is the a DeclVars tree of globals variables
    self->globals.type = SYMBOL_TABLE_GLOBAL;
    SymbolTable_create_from_DeclVars(&self->globals, 0, tree->firstChild);

    // tree->firstChild->nextSibling is the first function to process
    _ProgramSymbolTable_from_DeclFoncts(self, tree->firstChild->nextSibling);

    return 0;
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
