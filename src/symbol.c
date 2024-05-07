#include "symbol.h"

#include <stdio.h>
#include <string.h>

int Symbol_cmp(const void* a, const void* b) {
    return strcmp(((Symbol*)a)->identifier, ((Symbol*)b)->identifier);
}

const char* Symbol_get_type_str(type_t type) {
    static const char* types[] = {
        [type_byte] = "char",
        [type_num] = "int",
        [type_void] = "void",
    };

    return types[type];
}

const char* SymbolType_to_str(SymbolType type) {
    static const char* types[] = {
        [SYMBOL_VALUE] = "value",
        [SYMBOL_ARRAY] = "array",
        [SYMBOL_FUNCTION] = "function",
    };

    return types[type];
}

static const char* _Symbol_get_location_str(const Symbol* self) {
    static char addr_str[256];
    snprintf(addr_str, 256, "%d", self->addr);
    return addr_str;
}

static void _Symbol_print_Function(const Symbol* self) {
    printf(
        "%-15s : symbol_type=%-16s type=%-5s index=%-2d\n",
        self->identifier,
        SymbolType_to_str(self->symbol_type),
        Symbol_get_type_str(self->type),
        self->index);
}

static void _Symbol_print_Array(const Symbol* self) {
    printf(
        "%-15s : symbol_type=%-16s type=%-5s length=%d total_size=%d index=%-2d have_length=%s",
        self->identifier,
        SymbolType_to_str(self->symbol_type),
        Symbol_get_type_str(self->type),
        self->array.length,
        self->total_size,
        self->index,
        self->array.have_length ? "true" : "false");
    printf(
        " addr=%s",
        _Symbol_get_location_str(self));
    if (self->array.have_length) {
        printf(" length=%d", self->array.length);
    }
    putchar('\n');
}

static void _Symbol_print_Value(const Symbol* self) {
    printf(
        "%-15s : symbol_type=%-16s type=%-5s total_size=%d index=%-2d",
        self->identifier,
        SymbolType_to_str(self->symbol_type),
        Symbol_get_type_str(self->type),
        self->total_size,
        self->index);
    printf(
        "addr=%s",
        _Symbol_get_location_str(self));
    putchar('\n');
}

void Symbol_print(const Symbol* self) {
    const void (*printers[])(const Symbol*) = {
        [SYMBOL_VALUE] = _Symbol_print_Value,
        [SYMBOL_ARRAY] = _Symbol_print_Array,
        [SYMBOL_FUNCTION] = _Symbol_print_Function,
    };

    printers[self->symbol_type](self);
}
