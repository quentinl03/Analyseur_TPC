/**
 * @file table.c
 * @author Laborde Quentin
 * @brief
 * @date 31-01-2024
 *
 */

#include "table.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Initialize a symbol table */
bool STable_init(Symbol_Table *table) {
    table->tab = malloc(sizeof(Cell) * TABLE_INIT_SIZE);
    if (!table->tab) {
        return false;
    }
    table->max_len = TABLE_INIT_SIZE;
    table->real_len = 0;
    return true;
}

/**
 * @brief Reallocation of the table.
 *
 * @param table The table to reallocate.
 */
static void _stable_realloc(Symbol_Table *table) {
    table->tab = realloc(table->tab, sizeof(Cell) * table->max_len *
                                         TABLE_REALLOC_MULT);
    if (!table->tab) {
        fprintf(stderr, "Error: realloc failed\n");
        exit(1);
    }
    table->max_len *= TABLE_REALLOC_MULT;
}

/**
 * @brief Add the size of the cell according to its type.
 *
 * @param table The table to add the size.
 * @param size Quantity to add (array or 1).
 */
static void _stable_add_size(Symbol_Table *table, int size) {
    switch (table->tab[table->real_len].type) {
        case CHAR:
            table->tab[table->real_len].size = 1 * size;
            break;
        case INT:
            table->tab[table->real_len].size = 4 * size;
            break;

        default:
            fprintf(stderr, "Error: unknown type\n");
            exit(1);
            break;
    }
}
/* Add a symbol to the table */
void STable_add(Symbol_Table *table, char *name, Symbol_Type type, int size) {
    if (table->real_len + 1 == table->max_len) {
        _stable_realloc(table);
    }

    strncpy(table->tab[table->real_len].name, name, 64);
    table->tab[table->real_len].type = type;

    _stable_add_size(table, size);
    if (table->real_len == 0) {
        table->tab[table->real_len].addr = 0;
    }
    table->tab[table->real_len].addr = table->tab[table->real_len - 1].addr +
                                       table->tab[table->real_len - 1].size;

    table->real_len++;
}

/* Check the presence of a symbol */
bool STable_is_in(Symbol_Table *table, char *name) {
    for (int i = 0; i < table->real_len; i++) {
        if (!strcmp(table->tab[i].name, name)) {
            return true;
        }
    }
    return false;
}

/* Print the table */
void STable_print(Symbol_Table *table) {
    printf("Symbol table:\n");
    for (int i = 0; i < table->real_len; i++) {
        printf("name: %s, type: %d, addr: %p, size: %d\n", table->tab[i].name, table->tab[i].type,
               table->tab[i].addr, table->tab[i].size);
    }
}

/* Free the table */
void STable_free(Symbol_Table *table) {
    free(table->tab);
}

/* Change the real_len to make the table looks empty */
void STable_set_empty(Symbol_Table *table) {
    table->real_len = 0;
}

// int main(int argc, char const *argv[]) {
//     Symbol_Table table;
//     STable_init(&table);
//     STable_add(&table, "a", INT, 1);
//     STable_add(&table, "b", INT, 1);
//     STable_add(&table, "tab", CHAR, 100);
//     STable_add(&table, "d", INT, 1);
//     STable_print(&table);
//     return 0;
// }