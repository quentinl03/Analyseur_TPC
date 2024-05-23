#ifndef TPC_BISON_H
#define TPC_BISON_H

#include <stdio.h>

#include "tree.h"
#include "error.h"

/**
 * @brief Run the bison parser
 * 
 * @param f Input file (TPC source code)
 * @param abr Generated Abstract Syntax Tree
 * @return ErrorType 
 * - ERR_PARSE_SYNTAX if the syntax is incorrect
 * - ERR_NO_MEMORY if there is not enough memory
 * - ERR_NONE else
 */
ErrorType parser_bison(FILE* f, Node** abr);

#endif
