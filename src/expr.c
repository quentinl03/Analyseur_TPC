#include "expr.h"

static const char* INSTR_LOAD_LEAF_VALUE =
    "; Load leaf value\n"
    "mov rax, qword [rbp + %d]\n"
    "push rax\n"
    "\n";

static const char* INSTR_LITERAL_LEAF_VALUE =
    "; Load leaf value\n"
    "mov rax, %d\n"
    "push rax\n"
    "\n";

/* %s is the operator (add, sub, mul, div) */
static const char* INSTR_APPLY_OPERATOR =
    "; Apply operator\n"
    "pop rcx\n"
    "pop rax\n"
    "%s rax, rcx\n"
    "push rax\n"
    "\n";

static const char* MAP_OP_TO_INSTR[256] = {
    ['+'] = "add",
    ['-'] = "sub",
    ['*'] = "mul",
    ['/'] = "div",
    ['%'] = NULL, // TODO
};

int Expr_parse(FILE* output, const Tree tree) {

    if (IS_CONSTANT_LITERAL_LEAF(tree->label)) {
        fprintf(output, INSTR_LITERAL_LEAF_VALUE, tree->att.num);
        return 0;
    }

    if (IS_OPERATOR(tree->label)) {
        char operator = tree->att.byte;

        Expr_parse(output, tree->firstChild);
        fprintf(output, INSTR_APPLY_OPERATOR, MAP_OP_TO_INSTR[(int) operator]);

        return 0;
    }

    return 0;
}
