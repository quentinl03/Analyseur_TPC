#include "error.h"
#include "symboltable.h"
#include "tree.h"

typedef struct ExprReturn {
    ErrorType err;
    type_t type;  // Type of the expression (int / char)
} ExprReturn;

/**
 * @brief Check semantic vailidity before producing assembler
 *
 * @param tree Prog node
 * @param prog filled ProgramSymbolTable
 * @return ErrorType
 */
ErrorType Semantic_check(Tree tree, const ProgramSymbolTable* prog);