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

void CodeWriter_Init_File(FILE* nasm) {
    fprintf(nasm,
            "section .text\n"
            "global _start\n"
            "extern show_registers\n"
            "extern show_stack\n"
            "_start:\n");
}

void CodeWriter_End_File(FILE* nasm) {
    fprintf(nasm,
            "pop rbx\n"
            "call show_registers\n"
            "mov rax, 60\n"
            "mov rdi, 0\n"
            "syscall\n");
}

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
    const char* ope = _CodeWriter_Node_To_Ope(node);
    fprintf(nasm,
            "\n\n;Operation basique sur les 2 dernieres valeurs de la pile\n"
            "pop rcx\n"
            "pop rax\n"
            "%s rax, rcx\n"
            "push rax\n",
            ope);
    return 0;
}

int CodeWriter_ConstantNumber(FILE* nasm, const Node* node) {
    fprintf(nasm,
            "\n\n; Ajout constant numérique sur la pile\n"
            "push %d\n",
            node->att.num);
    return 0;
}

int CodeWriter_ConstantCharacter(FILE* nasm, const Node* node) {
    fprintf(nasm,
            "\n\n; Ajout constant character sur la pile\n"
            "push %d\n",
            node->att.byte);
    return 0;
}