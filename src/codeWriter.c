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

#include "registers.h"
#include "symbol.h"
#include "symboltable.h"
#include "tree.h"
#include "treeReader.h"

static void CodeWriter_entrypoint(FILE* nasm) {
    fprintf(
        nasm,
        "_start:\n"
        "call main\n"
        "mov rbx, rax\n"  //  ! TODO : A retirer version finale (print valuer retour main)
        "call show_registers\n"
        "mov rax, 60\n"               /* _exit() */
        "mov rdi, 0\n" /*Exit code*/  // TODO : Return main()'s exit code
        "syscall\n\n");
}

void CodeWriter_Init_File(FILE* nasm, const SymbolTable* globals) {
    assert(
        globals->type == SYMBOL_TABLE_GLOBAL &&
        "SymbolTable should be the program's global");

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
        "    global_vars: resb %ld\n\n"
        "section .text\n\n",
        globals->next_addr);
    CodeWriter_entrypoint(nasm);
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
    fprintf(
        nasm,
        "; Operation basique sur les 2 dernieres valeurs de la pile\n"
        "pop rcx\n"
        "pop rax\n"
        "%s rax, rcx\n"
        "push rax\n\n",
        ope);
    return 0;
}

int CodeWriter_ConstantNumber(FILE* nasm, const Node* node) {
    fprintf(
        nasm,
        "; Ajout d'une constante numérique sur la pile\n"
        "push %d\n\n",
        node->att.num);
    return 0;
}

int CodeWriter_ConstantCharacter(FILE* nasm, const Node* node) {
    fprintf(
        nasm,
        "; Ajout d'un caractère litéral sur la pile\n"
        "push %d\n\n",
        node->att.byte);
    return 0;
}

/**
 * @brief Emit code to call a function with its arguments
 *
 * @param nasm File to write to
 * @param node Function node (Ident node with EmptyArgs or ListExp node)
 * @return int
 */
static ErrorType CodeWriter_CallFunction(FILE* nasm,
                                         Node* node,
                                         const ProgramSymbolTable* symtable,
                                         const FunctionSymbolTable* func,
                                         const Symbol* symbol) {
    assert(
        node->label == Ident &&
        "Node should be an Ident node (function call)");

    fprintf(nasm, ";;; Appel de la fonction %s ;;;\n", symbol->identifier);

    if (node->firstChild->label != EmptyArgs) {
        // Call function with arguments
        int i = 0;
        for (Node* arg = node->firstChild->firstChild; arg;
             arg = arg->nextSibling, ++i) {
            TreeReader_Expr(symtable, arg, nasm, func);
            if (i < 6) {
                fprintf(
                    nasm,
                    "; Passage de l'argument %d à la fonction\n"
                    "pop %s\n",
                    i,
                    Register_to_str(Register_param_to_reg(i)));
            } else {
                assert(
                    0 &&
                    "Revoir le passage d'arguments > 6 (ordre d'empilement)");
                // TODO : Revoir le passage d'arguments > 6
            }
        }
    }

    fprintf(nasm, "call %s\n", symbol->identifier);

    // Push result on stack
    fprintf(
        nasm,
        "; Push valeur de retour sur la pile\n"
        "push rax\n");

    fprintf(
        nasm, ";;; Fin de l'appel de la fonction %s ;;;\n\n", symbol->identifier);

    return ERR_NONE;
}

int CodeWriter_LoadVar(FILE* nasm,
                       Node* node,
                       const ProgramSymbolTable* symtable,
                       const FunctionSymbolTable* func) {
    // TODO : Verif si la fucntion marche
    // Get in global variables
    const Symbol* symbol;
    symbol = SymbolTable_resolve_from_node(symtable, func, node);

    if (symbol->symbol_type == SYMBOL_FUNCTION) {
        CodeWriter_CallFunction(nasm, node, symtable, func, symbol);
    } else if (symbol->symbol_type == SYMBOL_VALUE) {
        if (symbol->is_static) {
            fprintf(
                nasm,
                "; Chargement de la variable globale '%s' sur la tête de pile\n"
                "push qword [global_vars + %d]\n",
                symbol->identifier,
                symbol->addr);
        } else {
            if (symbol->is_param) {
                fprintf(
                    nasm,
                    "; Chargement de l'argument '%s' sur la tête de pile\n"
                    "push %s\n",
                    symbol->identifier,
                    Register_to_str(symbol->reg));
            } else {
                fprintf(
                    nasm,
                    "; Chargement de la variable locale '%s' sur la tête de pile\n"
                    "push qword [rbp - %d]\n",
                    symbol->identifier,
                    symbol->addr + 8);
            }
        }
    } else {
        assert(0 && "Arrays not implemented, or we should't be there");
    }

    return 0;
}

int CodeWriter_WriteVar(FILE* nasm, Node* node,
                        const ProgramSymbolTable* symtable,
                        const FunctionSymbolTable* func) {
    // TODO : Verif si la fucntion marche

    const Symbol* symbol;
    symbol = SymbolTable_resolve_from_node(symtable, func, node);

    if (symbol->is_static) {
        fprintf(
            nasm,
            "; Assignation de la dernière valeur de la pile dans la variable globale '%s'\n"
            "pop rax\n"
            "mov [global_vars + %d], rax\n",
            symbol->identifier,
            symbol->addr);
    }

    else {
        if (symbol->is_param) {
            fprintf(
                nasm,
                "; Assignation de la dernière valeur de la pile dans l'argument '%s'\n"
                "pop rax\n"
                "mov %s, rax\n",
                symbol->identifier,
                Register_to_str(symbol->reg));
        } else {
            fprintf(
                nasm,
                "; Assignation de la dernière valeur de la pile dans la variable "
                "locale '%s'\n"
                "pop rax\n"
                "mov [rbp - %d], rax\n",
                symbol->identifier,
                symbol->addr + 8);
        }
    }

    // Pop last value
    // fprintf(
    //     nasm,
    //     "; Pop la dernière valeur de la pile après son assignation\n"
    //     "add rsp, %d\n\n",
    //     symbol->total_size
    // );

    return 0;
}

int CodeWriter_stackFrame_start(FILE* nasm, const FunctionSymbolTable* func) {
    // TODO : Implement > 6 functions paramters (compute sum of size of
    // paramters from index 6)
    fprintf(
        nasm,
        "; Init stack frame (save base pointer)\n"
        "push rbp\n"
        "mov rbp, rsp\n"
        "; Allocates %ld bytes on the the stack\n"
        "sub rsp, %ld\n\n",
        func->locals.next_addr,
        func->locals.next_addr);

    return 0;
}

int CodeWriter_stackFrame_end(FILE* nasm, const FunctionSymbolTable* func) {
    fprintf(
        nasm,
        "; Frees stack frame, (reset stack pointer to caller's state)\n"
        "mov rsp, rbp\n"
        "pop rbp\n\n");

    return 0;
}

int CodeWriter_FunctionLabel(FILE* nasm, const FunctionSymbolTable* func) {
    fprintf(nasm, "%s:\n\n", func->identifier);

    return 0;
}

int CodeWriter_Return(FILE* nasm) {
    fprintf(nasm, "ret\n\n");

    return 0;
}

int CodeWriter_Return_Expr(FILE* nasm) {
    fprintf(nasm, "pop rax\n");

    return 0;
}
