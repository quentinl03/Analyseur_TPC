#include "symbol.h"
#include <string.h>
#include <stdio.h>

int Symbol_init(Symbol* self, char* identifier, type_t type, size_t size) {
    *self = (Symbol) {
        .identifier = identifier,
        .type = type,
        .size = size,
    };

    return 0;
}

int Symbol_cmp(const void* a, const void* b) {
    return strcmp(((Symbol*) a)->identifier, ((Symbol*) b)->identifier);
}

static const char* _Symbol_get_type_str(type_t type) {
    static const char* types[] = {
        [type_byte] = "char",
        [type_num] = "int",
    };

    return types[type];
}

void Symbol_print(const Symbol* symbol) {
    printf(
        "Symbol: %-10s : type=%-5s, size=%-3lu, addr=%#04lx\n",
        symbol->identifier,
        _Symbol_get_type_str(symbol->type),
        symbol->size,
        symbol->addr
    );
}
