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
            "\npop rbx\n"
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

int CodeWriter_LoadVar(FILE* nasm, Node* node,
                       const ProgramSymbolTable* symtable, char* func_name) {
    // TODO : Verif si la fucntion marche
    // Get in global variables
    Symbol* symbol = SymbolTable_get(&symtable->globals, node->att.ident);

    // Get in local variables (overwrites global variable)
    FunctionSymbolTable* func_table = SymbolTable_get_from_func_name(symtable, func_name);

    Symbol* tmp;
    if ((tmp = SymbolTable_get(&func_table->parameters, node->att.ident))) {
        symbol = tmp;
    } else if ((tmp = SymbolTable_get(&func_table->locals, node->att.ident))) {
        symbol = tmp;
    }

    assert(symbol && "Variable not found");

    fprintf(nasm,
            "\n\n; Ajout d'un contenu de variable sur la pile\n"
            "mov rax, [rsp + %d]\n"
            "push rax\n",
            symbol->addr);
    return 0;
}

int CodeWriter_WriteVar(FILE* nasm, Node* node,
                        const ProgramSymbolTable* symtable, char* func_name) {
    // TODO : Verif si la fucntion marche
    // Get in global variables
    Symbol* symbol = SymbolTable_get(&symtable->globals, node->att.ident);

    // Get in local variables (overwrites global variable)
    FunctionSymbolTable* func_table = SymbolTable_get_from_func_name(symtable, func_name);

    Symbol* tmp;
    if ((tmp = SymbolTable_get(&func_table->parameters, node->att.ident))) {
        symbol = tmp;
    } else if ((tmp = SymbolTable_get(&func_table->locals, node->att.ident))) {
        symbol = tmp;
    }

    assert(symbol && "Variable not found");

    fprintf(nasm,
            "\n\n; Assignation de la derniere valuer de la pile dans une variable\n"
            "pop rax\n"
            "mov [rsp + %d], rax\n",
            symbol->addr);
    return 0;
}