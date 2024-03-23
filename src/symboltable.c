#include "symboltable.h"

#include <string.h>
#include <stdio.h>
#include <assert.h>

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
    *self = (SymbolTable) {0};
    return ArrayList_init(&self->symbols, sizeof(Symbol), 30, Symbol_cmp);
}

/**
 * @brief Initialize a FunctionSymbolTable object
 * 
 * @param self FunctionSymbolTable object
 * @param identifier Function name, should be pre-allocated
 */
static void _FunctionSymbolTable_init(FunctionSymbolTable* self, char* identifier) {
    *self = (FunctionSymbolTable) {
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

    if (self->type == SYMBOL_TABLE_PARAM && ArrayList_get_length(&self->symbols) < 6) {
        symbol.on_register = true;
        symbol.reg = Register_param_to_reg(ArrayList_get_length(&self->symbols));
    }
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

Symbol* SymbolTable_get(SymbolTable* self, char* identifier) {
    Symbol symbol = (Symbol) {
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
static size_t get_type_size(type_t type) {
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
static type_t get_type_from_string(const Attribut* att) {
    if (strcmp(att->key_word, "char") == 0) {
        return type_byte;
    }
    else if (strcmp(att->key_word, "int") == 0) {
        return type_num;
    }

    return -1;
}

/**
 * @brief Create a symbol table from a DeclVar tree
 * (used for global variables and function parameters)
 * Shouldn't be called directly, use SymbolTable_create_from_DeclVar instead
 * 
 * @param self SymbolTable object to fill
 * @param tree DeclVar tree
 * @param type Type of the variables 
 * @return int 
 */
static int _SymbolTable_create_from_DeclVar(SymbolTable* self, Tree tree, type_t type) {
    if (!tree) {
        return -1;
    }

    if (tree->label == Type) {
        _SymbolTable_create_from_DeclVar(self, tree->firstChild, get_type_from_string(&tree->att));
    }

    else if (tree->label == DeclArray) {
        _SymbolTable_add(
            self,
            (Symbol) {
                .identifier = tree->att.ident,
                .is_static = self->type == SYMBOL_TABLE_GLOBAL,
                .type = type,
                .type_size = get_type_size(type),
                .symbol_type = SYMBOL_ARRAY,
                .array.length = tree->firstChild->att.num,
                .total_size = tree->firstChild->att.num * get_type_size(type),
            }
        );
    }

    else if (tree->label == Ident) {
        _SymbolTable_add(
            self,
            (Symbol) {
                .identifier = tree->att.ident,
                .is_static = self->type == SYMBOL_TABLE_GLOBAL,
                .type = type,
                .type_size = get_type_size(type),
                .symbol_type = SYMBOL_VALUE,
                .total_size = 1 * get_type_size(type),
            }
        );
    }

    _SymbolTable_create_from_DeclVar(self, tree->nextSibling, type);

    return 1;
}

/**
 * @brief Create symbol table from a DeclVar tree
 * 
 * @param self SymbolTable object
 * @param offset Offset to start adding variables 
 * (functions parameters could be stored on the stack if they are more than 6)
 * @param tree Tree of DeclVars containing variables
 * @return int 
 */
static int SymbolTable_create_from_DeclVar(SymbolTable* self, int offset, Tree tree) {
    self->next_addr = offset;
    return _SymbolTable_create_from_DeclVar(self, tree->firstChild, -1);
}

/**
 * @brief Add function parameters to the symbol table
 * Begins exploration from ListTypVar
 * 
 * @param func FunctionSymbolTable object to fill
 * @param tree Tree ListTypVar tree
 * @return int 
 */
static int _FunctionSymbolTable_create_from_EnTeteFonct(FunctionSymbolTable* func, Tree tree) {
    if (!tree) {
        return 0;
    }

    if (tree->label == ListTypVar) {
        _FunctionSymbolTable_create_from_EnTeteFonct(func, tree->firstChild);
    }

    else if (tree->label == Type) {
        _SymbolTable_create_from_DeclVar(&func->parameters, tree->firstChild, get_type_from_string(&tree->att));
    }

    else if (tree->label == DeclFonctArray) {
        type_t type = get_type_from_string(&tree->att);
        _SymbolTable_add(
            &func->parameters,
            (Symbol) {
                .identifier = tree->firstChild->att.ident,
                .type = type,
                .type_size = get_type_size(type),
                .symbol_type = SYMBOL_POINTER_TO_ARRAY,
                .is_static = false,

                // TODO: Is this ok?
                .total_size = 8, // An array is decayed to a pointer in C (8 bytes on x86_64 systems)
            }
        );
    }

    return _FunctionSymbolTable_create_from_EnTeteFonct(func, tree->nextSibling);
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

    _FunctionSymbolTable_init(func, header->firstChild->nextSibling->att.ident);

    type_t return_type = is_void ? type_void : get_type_from_string(&header->firstChild->att);

    // * Add the function to the program's symbol table
    _SymbolTable_add(
        &prog->globals,
        (Symbol) {
            // function name
            .identifier = header->firstChild->nextSibling->att.ident,
            .symbol_type = SYMBOL_FUNCTION,
            // return type
            .is_static = true,
            .type = return_type,
            .type_size = get_type_size(return_type),
            .total_size = 0,
        }
    );

    // * Add the function's parameters to the function's symbol table
    _FunctionSymbolTable_create_from_EnTeteFonct(func, header->firstChild->nextSibling->nextSibling);
    
    // * Add the function's local variables to the function's symbol table
    int offset = func->parameters.next_addr; // Parameters could be stored on the stack
    // DeclFonct->EnTeteFonct->Corps->DeclVars
    SymbolTable_create_from_DeclVar(&func->locals, offset, tree->firstChild->nextSibling->firstChild);

    return 0;
}

/**
 * @brief Starts tree exploration from a DeclFoncts or a DeclFonct
 * and creates a symbol table for each function
 * 
 * @param self ProgramSymbolTable object to fill
 * @param tree Tree to explore, should be a DeclFoncts or a DeclFonct
 * @return int 
 */
static int _ProgramSymbolTable_from_DeclFoncts(ProgramSymbolTable* self, Tree tree) {
    if (!tree) {
        return -1;
    }

    if (tree->label == DeclFoncts) {
        _ProgramSymbolTable_from_DeclFoncts(self, tree->firstChild);
    } 

    else if (tree->label == DeclFonct) {
        FunctionSymbolTable function;
        _FunctionSymbolTable_init(&function, tree->firstChild->firstChild->nextSibling->att.ident);

        _SymbolTable_create_from_DeclFonct(self, &function, tree);

        ArrayList_append(&self->functions, &function);
    }

    _ProgramSymbolTable_from_DeclFoncts(self, tree->nextSibling);

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
    *self = (ProgramSymbolTable) {0};
    _SymbolTable_init(&self->globals);
    ArrayList_init(&self->functions, sizeof(FunctionSymbolTable), 10, NULL);

    // tree->firstChild is the a DeclVars tree of globals variables
    self->globals.type = SYMBOL_TABLE_GLOBAL;
    SymbolTable_create_from_DeclVar(&self->globals, 0, tree->firstChild);

    // tree->firstChild->nextSibling is the first function to process
    _ProgramSymbolTable_from_DeclFoncts(self, tree->firstChild->nextSibling);

    return 0;
}

void SymbolTable_print(const SymbolTable* self) {
    for (int i = 0; i < ArrayList_get_length(&self->symbols); i++) {
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
