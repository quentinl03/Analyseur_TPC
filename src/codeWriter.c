/**
 * @file CodeWriter.c
 * @author Laborde Quentin & Seban Nicolas
 * @brief
 * @date 27-03-2024
 *
 * @copyright Copyright (c) 2024
 *
 */

#include "codeWriter.h"

#include <assert.h>
#include <stdio.h>

#include "symboltable.h"
#include "tree.h"

static const char* _CodeWriter_Node_To_Ope(const Node* node) {
    switch (node->att.byte) {
        case '+':
            return "add";
        case '-':
            return "sub";
        case '*':
            return "imul";
        case '/':
            return "idiv";
        default:
            assert(0 && "We shoudn't be there");
    }
}

int CodeWriter_Ope(FILE* nasm, const Node* node) {
    fprintf(nasm,
            "pop rcx\n"
            "pop rax\n"
            "%s rax, rcx\n"
            "push rax\n",
            _CodeWriter_Node_To_Ope(node));
    return 0;
}

int CodeWriter_ConstantNumber(FILE* nasm, const Node* node) {
    fprintf(nasm,
            "push %d\n",
            node->att.num);
    return 0;
}