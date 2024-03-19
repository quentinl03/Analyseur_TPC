#include "symboltable.h"

#include <string.h>
#include <stdio.h>
#include <assert.h>

#define BOLD "\x1b[1m"
#define UNDERLINE "\x1b[4m"
#define RESET "\x1b[0m"


int SymbolTable_init(SymbolTable* self) {
    *self = (SymbolTable) {0};
    return ArrayList_init(&self->symbols, sizeof(Symbol), 30, Symbol_cmp);
}

void FunctionSymbolTable_init(FunctionSymbolTable* self, char* identifier) {
    *self = (FunctionSymbolTable) {
        .identifier = identifier,
    };
    SymbolTable_init(&self->parameters);
    SymbolTable_init(&self->locals);
}

int SymbolTable_add(SymbolTable* self, Symbol symbol) {
    if (ArrayList_contains(&self->symbols, &symbol))
        return 1;

    ArrayListError err = 0;

    symbol.addr = self->next_addr;
    if ((err = ArrayList_sorted_insert(&self->symbols, &symbol)) < -1) {
        return -1;
    }

    self->next_addr += symbol.size;

    return 0;

}

Symbol* SymbolTable_get(SymbolTable* self, char* identifier) {
    Symbol symbol = (Symbol) {
        .identifier = identifier,
    };

    return ArrayList_search(&self->symbols, &symbol);
}

static size_t _SymbolTable_get_size(type_t type) {
    static const size_t sizes[] = {
        [type_byte] = sizeof(char),
        [type_num] = sizeof(int),
        [type_void] = 0,
    };

    return sizes[type];
}

static type_t _SymbolTable_get_type(const Attribut* att) {
    if (strcmp(att->key_word, "char") == 0) {
        return type_byte;
    }
    else if (strcmp(att->key_word, "int") == 0) {
        return type_num;
    }

    return -1;
}

static int _SymbolTable_create_from_DeclVar(SymbolTable* self, Tree tree, type_t type) {
    if (!tree) {
        return -1;
    }

    if (tree->label == Type) {
        _SymbolTable_create_from_DeclVar(self, tree->firstChild, _SymbolTable_get_type(&tree->att));
    }

    else if (tree->label == DeclArray) {
        SymbolTable_add(
            self,
            (Symbol) {
                .identifier = tree->att.ident,
                .size = tree->firstChild->att.num * _SymbolTable_get_size(type),
                .type = type,
            }
        );
    }

    else if (tree->label == Ident) {
        SymbolTable_add(
            self,
            (Symbol) {
                .identifier = tree->att.ident,
                .size = 1 * _SymbolTable_get_size(type),
                .type = type,
            }
        );
    }

    _SymbolTable_create_from_DeclVar(self, tree->nextSibling, type);

    return 1;
}

int SymbolTable_from_DeclVar(SymbolTable* self, Tree tree) {
    return _SymbolTable_create_from_DeclVar(self, tree->firstChild, -1);
}

void SymbolTable_print(const SymbolTable* self) {
    for (size_t i = 0; i < ArrayList_get_length(&self->symbols); i++) {
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
    printf(UNDERLINE "Globals:\n\n" RESET);
    SymbolTable_print(&self->globals);
    printf(UNDERLINE "Functions:\n\n" RESET);
    for (int i = 0; i < ArrayList_get_length(&self->functions); i++) {
        FunctionSymbolTable* function = ArrayList_get(&self->functions, i);
        FunctionSymbolTable_print(function);
        putchar('\n');
    }
}

/**
 * @brief Add function parameters to the symbol table
 * Begins exploration ListTypVar
 * 
 * @param func FunctionSymbolTable object
 * @param tree ListTypVar tree
 * @return int 
 */
static int create_from_EnTeteFonct_aux_ListTypVar(FunctionSymbolTable* func, Tree tree) {
    if (!tree) {
        return 0;
    }

    if (tree->label == ListTypVar) {
        create_from_EnTeteFonct_aux_ListTypVar(func, tree->firstChild);
    }

    else if (tree->label == Type) {
        _SymbolTable_create_from_DeclVar(&func->parameters, tree->firstChild, _SymbolTable_get_type(&tree->att));
    }

    else if (tree->label == DeclFonctArray) {
        SymbolTable_add(
            &func->parameters,
            (Symbol) {
                .identifier = tree->firstChild->att.ident,
                .type = _SymbolTable_get_type(&tree->att),
                // ? TODO
                .size = 8, // An array is decayed to a pointer in C (8 bytes on x86_64 systems)
            }
        );
    }

    return create_from_EnTeteFonct_aux_ListTypVar(func, tree->nextSibling);
}

/**
 * @brief Create a symbol table from a EnTeteFonct tree
 * 
 * @param prog SymbolTable of the program
 * @param func FunctionSymbolTable of the function
 * @param func DeclFonct tree
 * @return int 
 */
static int SymbolTable_create_from_DeclFonct(ProgramSymbolTable* prog, FunctionSymbolTable* func, Tree tree) {
    if (!tree) {
        return -1;
    }

    // Go to the EnTeteFonct tree
    Node* header = tree->firstChild;

    assert(tree->label == DeclFonct);
    bool is_void = header->firstChild->type == type_void;

    // * Add the function to the program's symbol table
    SymbolTable_add(
        &prog->globals,
        (Symbol) {
            // function name
            .identifier = header->firstChild->nextSibling->att.ident,
            // return type
            .type = is_void ? type_void : _SymbolTable_get_type(&header->firstChild->att),
            .size = is_void ? 0 : _SymbolTable_get_size(_SymbolTable_get_type(&header->firstChild->att)),
        }
    );

    // * Add the function's parameters to the function's symbol table
    create_from_EnTeteFonct_aux_ListTypVar(func, header->firstChild->nextSibling->nextSibling);
    
    // * Add the function's local variables to the function's symbol table
    // DeclFonct->EnTeteFonct->Corps->DeclVars
    SymbolTable_from_DeclVar(&func->locals, tree->firstChild->nextSibling->firstChild);

    return 0;
}

/**
 * @brief Starts tree exploration from a DeclFoncts or a DeclFonct
 * 
 * @param self 
 * @param tree 
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
        FunctionSymbolTable_init(&function, tree->firstChild->firstChild->nextSibling->att.ident);

        SymbolTable_create_from_DeclFonct(self, &function, tree);

        ArrayList_append(&self->functions, &function);
    }

    _ProgramSymbolTable_from_DeclFoncts(self, tree->nextSibling);

    return 1;

}

int ProgramSymbolTable_from_Prog(ProgramSymbolTable* self, Tree tree) {
    *self = (ProgramSymbolTable) {0};
    SymbolTable_init(&self->globals);
    ArrayList_init(&self->functions, sizeof(FunctionSymbolTable), 10, NULL);

    // tree->firstChild is the a DeclVars tree of globals variables
    SymbolTable_from_DeclVar(&self->globals, tree->firstChild);

    // tree->firstChild->nextSibling is the first function to process
    _ProgramSymbolTable_from_DeclFoncts(self, tree->firstChild->nextSibling);

    return 0;
}
