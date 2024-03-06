/**
 * @file table.h
 * @author Laborde Quentin
 * @brief
 * @date 31-01-2024
 *
 */

#ifndef __TABLE_H__
#define __TABLE_H__

#include <stdbool.h>

#define TABLE_INIT_SIZE 10
#define TABLE_REALLOC_MULT 2

/**************/
/* Structures */
/**************/

typedef enum {
    CHAR,
    INT,
} Symbol_Type;

typedef struct {
    int size;
    Symbol_Type type;
    void* addr;
    char name[64];
} Cell;

typedef struct {
    int real_len;
    int max_len;
    Cell* tab;
} Symbol_Table;

/*************/
/* Functions */
/*************/

/**
 * @brief Initialize a symbol table.
 *
 * @param table The table to initialize.
 * @return if the initialization was successful.
 * (error allocation possible)
 */
bool STable_init(Symbol_Table* table);

/**
 * @brief Add a symbol to the table.
 *
 * @param table The table to add the symbol.
 * @param name The name of the symbol.
 * @param type The type of the symbol.
 */
void STable_add(Symbol_Table* table, char* name, Symbol_Type type, int size);

/**
 * @brief Check if a symbol is in the table.
 *
 * @param table The table to check.
 * @param name The name of the symbol.
 * @return if the symbol is in the table.
 */
bool STable_is_in(Symbol_Table* table, char* name);

/**
 * @brief Print the table.
 *
 * @param table The table to print.
 */
void STable_print(Symbol_Table* table);

/**
 * @brief Free the table.
 *
 * @param table The table to free.
 */
void STable_free(Symbol_Table* table);

/**
 * @brief Change the size of the table to 0.
 *
 * @param table The table to set empty.
 */
void STable_set_empty(Symbol_Table* table);

#endif
