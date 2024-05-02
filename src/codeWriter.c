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
#include <string.h>

#include "registers.h"
#include "symbol.h"
#include "symboltable.h"
#include "tree.h"
#include "treeReader.h"

int GLOBAL_CMP;

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
        default:
            assert(0 && "We shoudn't be there");
    }
}

void CodeWriter_Ope(FILE* nasm, const Node* node) {
    if (node->att.byte == '/' || node->att.byte == '%') {
        fprintf(
            nasm,
            "; Operation basique sur les 2 dernieres valeurs de la pile\n"
            "mov rdx, 0\n"
            "pop rcx\n"
            "pop rax\n"
            "idiv rcx\n"
            "push %s\n\n",
            (node->att.byte == '%') ? "rdx" : "rax");
        return;
    }
    const char* ope = _CodeWriter_Node_To_Ope(node);
    fprintf(
        nasm,
        "; Operation basique sur les 2 dernieres valeurs de la pile\n"
        "pop rcx\n"
        "pop rax\n"
        "%s rax, rcx\n"
        "push rax\n\n",
        ope);
}

void CodeWriter_Ope_Unaire(FILE* nasm, const Node* node) {
    if (node->att.byte == '-') {
        fprintf(
            nasm,
            "; Operation oposé la derniere valeur de la pile\n"
            "pop rax\n"
            "neg rax\n"
            "push rax\n\n");
    }
}

void CodeWriter_ConstantNumber(FILE* nasm, const Node* node) {
    fprintf(
        nasm,
        "; Ajout d'une constante numérique sur la pile\n"
        "push %d\n\n",
        node->att.num);
}

void CodeWriter_ConstantCharacter(FILE* nasm, const Node* node) {
    fprintf(
        nasm,
        "; Ajout d'un caractère litéral sur la pile\n"
        "push %d\n\n",
        node->att.byte);
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
    assert(
        symbol->symbol_type == SYMBOL_FUNCTION &&
        "Symbol should be a function");
    assert(node->firstChild != NULL);

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

void CodeWriter_LoadVar(FILE* nasm,
                        Node* node,
                        const ProgramSymbolTable* symtable,
                        const FunctionSymbolTable* func) {
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
}

void CodeWriter_WriteVar(FILE* nasm, Node* node,
                         const ProgramSymbolTable* symtable,
                         const FunctionSymbolTable* func) {
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
}

void CodeWriter_stackFrame_start(FILE* nasm, const FunctionSymbolTable* func) {
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
}

void CodeWriter_stackFrame_end(FILE* nasm, const FunctionSymbolTable* func) {
    fprintf(
        nasm,
        "; Frees stack frame, (reset stack pointer to caller's state)\n"
        "mov rsp, rbp\n"
        "pop rbp\n\n");
}

void CodeWriter_FunctionLabel(FILE* nasm, const FunctionSymbolTable* func) {
    fprintf(nasm, "%s:\n\n", func->identifier);
}

void CodeWriter_Return(FILE* nasm) {
    fprintf(nasm, "ret\n\n");
}

void CodeWriter_Return_Expr(FILE* nasm) {
    fprintf(nasm, "pop rax\n");
}

static char* _CodeWriter_Node_To_Eq(const Node* node) {
    if (!strcmp(node->att.key_word, "==")) {
        return "je";
    }
    if (!strcmp(node->att.key_word, "!=")) {
        return "jne";
    }
    assert(0 && "We shoudn't be there (_CodeWriter_Node_To_Eq) (Unkonw cmp)");
}

static char* _CodeWriter_Node_to_Order(const Node* node) {
    switch (node->att.key_word[0]) {
        case '>':
            if (node->att.key_word[1] == '=') {
                return "jge";
            }
            return "jg";
        case '<':
            if (node->att.key_word[1] == '=') {
                return "jle";
            }
            return "jl";

        default:
            assert(0 && "We shouldn't be there (CodeWriter_Node_to_Order) (Wrong comparator)");
            break;
    }
}

void CodeWriter_Cmp(FILE* nasm, Node* node) {
    char* cmp = NULL;
    if (node->label == Eq) {
        cmp = _CodeWriter_Node_To_Eq(node);
    } else if (node->label == Order) {
        cmp = _CodeWriter_Node_to_Order(node);
    }
    assert(cmp && "Comparaison symbol unknow (CodeWriter_Cmp)");

    fprintf(
        nasm,
        "; Comparaison sur les 2 dernieres valeurs de la pile\n"
        "pop rcx\n"
        "pop rax\n"
        "cmp rcx, rax\n"
        "%s cmp_%d ; comparateur si vrai va dans 2e cas\n"
        "push 0\n"
        "jmp cmp_end%d\n"
        "cmp_%d :\n"
        "push 1\n"
        "jmp cmp_end%d\n"
        "cmp_end%d : \n\n",
        cmp, GLOBAL_CMP, GLOBAL_CMP, GLOBAL_CMP, GLOBAL_CMP, GLOBAL_CMP);
    GLOBAL_CMP++;
}

int CodeWriter_If_Init(FILE* nasm) {
    fprintf(
        nasm,
        "; Condition if_%d\n"
        "pop rax\n"
        "cmp rax, 0\n"
        "je else_%d\n; if case\n",
        GLOBAL_CMP, GLOBAL_CMP);
    return GLOBAL_CMP++;
}

void CodeWriter_If_Else(FILE* nasm, int if_number) {
    fprintf(
        nasm,
        "jmp end_if_%d\n"
        "else_%d :\n; else case\n",
        if_number, if_number);
}

void CodeWriter_If_End(FILE* nasm, int if_number) {
    fprintf(
        nasm,
        "end_if_%d :\n",
        if_number);
}

int CodeWriter_While_Init(FILE* nasm) {
    fprintf(
        nasm,
        "; Condition while_%d\n"
        "while_start_%d :\n; while eval cond\n",
        GLOBAL_CMP, GLOBAL_CMP);
    return GLOBAL_CMP++;
}

void CodeWriter_While_Eval(FILE* nasm, int while_number) {
    fprintf(
        nasm,
        "pop rax\n"
        "cmp rax, 0\n"
        "je end_while_%d\n ; while expr\n",
        while_number);
}

void CodeWriter_While_End(FILE* nasm, int while_number) {
    fprintf(
        nasm,
        "jmp while_start_%d\n"
        "end_while_%d :\n",
        while_number, while_number);
}