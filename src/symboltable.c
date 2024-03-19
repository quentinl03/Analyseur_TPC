#include "symboltable.h"

#include <string.h>
#include <stdio.h>

int SymbolTable_init(SymbolTable* self) {
    *self = (SymbolTable) {0};
    return ArrayList_init(&self->symbols, sizeof(Symbol), 30, Symbol_cmp);
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

static int _ProgramSymbolTable_from_DeclFoncts(ProgramSymbolTable* self, Tree tree) {
    if (!tree) {
        return -1;
    }

    if (tree->label == DeclFoncts) {
        _ProgramSymbolTable_from_DeclFoncts(self, tree->firstChild);
    } 

    else if (tree->label == DeclFonct) {
        FunctionSymbolTable function;
        SymbolTable_init(&function.parameters);

        SymbolTable_from_DeclVar(&function.locals, tree->firstChild->nextSibling->firstChild);

        ArrayList_append(&self->functions, &function);
    }

    _ProgramSymbolTable_from_DeclFoncts(self, tree->nextSibling);

    return 1;

}

int ProgramSymbolTable_from_Prog(ProgramSymbolTable* self, Tree tree) {
    *self = (ProgramSymbolTable) {0};
    SymbolTable_init(&self->globals);
    ArrayList_init(&self->functions, sizeof(SymbolTable), 10, NULL);

    SymbolTable_from_DeclVar(&self->globals, tree->firstChild);
    _ProgramSymbolTable_from_DeclFoncts(self, tree->firstChild->nextSibling);
}