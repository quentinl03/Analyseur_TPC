#include "codegen.h"
#include "expr.h"

static const char* ASM_HEADER = 
    ".section .data\n"
    ".section .text\n"
    ".global _start\n"
    "_start:\n";

static int _CodeGen_parse_SuiteInstr(FILE* file, const Tree tree) {
    if (IS_EMPTY_TREE(tree)) {
        return 0;
    }

    if (tree->label == Return) {
        printf("return\n");
        Expr_parse(file, tree);
    }

    return _CodeGen_parse_SuiteInstr(file, tree->nextSibling);
}

int CodeGen_parse(const char* output, const Tree tree, ProgramSymbolTable* symbolTable) {
    FILE* file = fopen(output, "w");
    if (!file) {
        perror("fopen");
        return 1;
    }

    fputs(ASM_HEADER, file);

    _CodeGen_parse_SuiteInstr(file, tree);

    fclose(file);

    return 0;
}
