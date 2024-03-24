#ifndef EXPR_H
#define EXPR_H

#include "tree.h"
#include <stdio.h>

#define IS_OPERATOR(NODE) (  \
    NODE == Addsub  || \
    NODE == Divstar || \
    NODE == Or      || \
    NODE == And     || \
    NODE == Eq      || \
    NODE == Order   || \
    NODE == Not        \
)

#define IS_CONSTANT_LITERAL_LEAF(NODE) (  \
    NODE == Num       || \
    NODE == Character    \
)

/**
 * @brief Emit assembly code to evaluate an expression
 * 
 * @param output Opened file to write ASM code to
 * @param tree An expression tree to evaluate (See IS_OPERATOR under src/tree.h)
 * @return int 
 */
int Expr_parse(FILE* output, const Tree tree);

#endif
