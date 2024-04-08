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

#include "symbol.h"
#include "symboltable.h"
#include "tree.h"

void CodeWriter_Init_File(FILE* nasm, const SymbolTable* globals) {
    assert(globals->type == SYMBOL_TABLE_GLOBAL && "SymbolTable should be the program's global");
    fprintf(
        nasm,
        "global _start\n"
        "extern show_registers\n"
        "extern show_stack\n"
        "extern putchar\n"
        "extern getchar\n"
        "extern putint\n"
        "extern getint\n"
        "section .bss\n"
        "    global_vars resb %ld\n\n"
        "section .text\n"
        "_start:\n\n",
        globals->next_addr
    );
}

void CodeWriter_End_File(FILE* nasm) {
    fprintf(nasm,
            "pop rbx\n"
            "call show_registers\n"
            "mov rax, 60\n"
            "mov rdi, 0\n"
            "syscall\n\n");
}

static const char* _CodeWriter_Node_To_Ope(const Node* node) {
    switch (node->att.byte) {
        case '+':
            return "add";
        case '-':
            return "sub";
        case '*':
            return "imul";
        case '/':  // ! TODO : A revoir (car idiv c de la merde)
            return "idiv";
        default:
            assert(0 && "We shoudn't be there");
    }
}

int CodeWriter_Ope(FILE* nasm, const Node* node) {
    const char* ope = _CodeWriter_Node_To_Ope(node);
    fprintf(nasm,
            "; Operation basique sur les 2 dernieres valeurs de la pile\n"
            "pop rcx\n"
            "pop rax\n"
            "%s rax, rcx\n"
            "push rax\n\n",
            ope);
    return 0;
}

int CodeWriter_ConstantNumber(FILE* nasm, const Node* node) {
    fprintf(nasm,
            "; Ajout constant numérique sur la pile\n"
            "push %d\n\n",
            node->att.num);
    return 0;
}

int CodeWriter_ConstantCharacter(FILE* nasm, const Node* node) {
    fprintf(nasm,
            "; Ajout constant character sur la pile\n"
            "push %d\n\n",
            node->att.byte);
    return 0;
}

int CodeWriter_LoadVar(
    FILE* nasm, Node* node,
    const ProgramSymbolTable* symtable,
    const FunctionSymbolTable* func
) {
    // TODO : Verif si la fucntion marche
    // Get in global variables
    Symbol* symbol = SymbolTable_get(&symtable->globals, node->att.ident);

    // Get in local variables (overwrites global variable)

    Symbol* tmp;
    if ((tmp = SymbolTable_get(&func->parameters, node->att.ident))) {
        symbol = tmp;
    } else if ((tmp = SymbolTable_get(&func->locals, node->att.ident))) {
        symbol = tmp;
    }

    assert(symbol && "Variable not found");

    fprintf(nasm,
            "; Ajout d'un contenu de variable sur la pile\n"
            "mov rax, [rsp + %d]\n" // TODO rbp ?
            "push rax\n\n",
            symbol->addr);
    return 0;
}

int CodeWriter_WriteVar(
    FILE* nasm, Node* node,
    const ProgramSymbolTable* symtable,
    const FunctionSymbolTable* func
) {
    // TODO : Verif si la fucntion marche
    // Get in global variables
    Symbol* symbol = SymbolTable_get(&symtable->globals, node->att.ident);

    // Get in local variables (overwrites global variable)

    Symbol* tmp;
    if ((tmp = SymbolTable_get(&func->parameters, node->att.ident))) {
        symbol = tmp;
    } else if ((tmp = SymbolTable_get(&func->locals, node->att.ident))) {
        symbol = tmp;
    }

    assert(symbol && "Variable not found");

    if (symbol->is_static) {
        fprintf(
            nasm,
            "; Assignation de la dernière valeur de la pile dans une variable\n"
            "mov [global_vars + %d], rsp\n\n",
            symbol->addr
        );
    }

    else {
        assert(0 && "Variable is not global / Not implemented"); // TODO : Implement assigment on locals
    }

    return 0;
}
