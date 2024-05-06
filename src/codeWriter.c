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

#define MIN(a, b) ((a) < (b) ? (a) : (b))

static void CodeWriter_entrypoint(FILE* nasm) {
    fprintf(
        nasm,
        "_start:\n"
        "call main\n"
        "mov rdi, rax\n"
        "mov rax, 60\n" /* _exit() */
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
        "push %d\n",
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
 * @brief Auxilliary function to call a function with its arguments,
 * in the opposite order.
 * Recursively go to the last argument, and while going back, evaluate
 * the arguments and push them on the stack.
 * 
 * @param nasm 
 * @param node 
 * @param symtable 
 * @param func 
 * @param symbol 
 */
static void _CodeWriter_CallFunction_aux(FILE* nasm,
                                         Node* node,
                                         const ProgramSymbolTable* symtable,
                                         const FunctionSymbolTable* func) {
    if (node == NULL) {
        return;
    }

    _CodeWriter_CallFunction_aux(nasm, node->nextSibling, symtable, func);

    TreeReader_Expr(symtable, node, nasm, func);
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
                                         const FunctionSymbolTable* caller,
                                         const Symbol* symbol) {
    assert(
        node->label == Ident &&
        "Node should be an Ident node (function call)");
    assert(
        symbol->symbol_type == SYMBOL_FUNCTION &&
        "Symbol should be a function");
    assert(node->firstChild != NULL);

    fprintf(nasm, ";;; Appel de la fonction %s ;;;\n", symbol->identifier);

    const FunctionSymbolTable* callee = FunctionSymbolTable_get_from_name(symtable, symbol->identifier);

    if (node->firstChild->label != EmptyArgs) {
        // Call function with arguments
        _CodeWriter_CallFunction_aux(nasm, node->firstChild->firstChild, symtable, caller);

        int nb_params = MIN(FunctionSymbolTable_get_param_count(callee), 6);

        for (int i = 0; i < nb_params; ++i) {
            const Symbol* param = FunctionSymbolTable_get_param(callee, i);
            fprintf(
                nasm,
                "pop %s\n",
                Register_to_str(Register_param_to_reg(i))
            );
        }
    }

    fprintf(
        nasm,
        //"and rsp, -16\n"
        "call %s\n", symbol->identifier
    );

    // Push result on stack
    fprintf(
        nasm,
        "; Push valeur de retour sur la pile\n"
        "push rax\n");

    fprintf(
        nasm, ";;; Fin de l'appel de la fonction %s ;;;\n\n", symbol->identifier);

    return ERR_NONE;
}

/**
 * @brief Load a function parameter on the stack
 * If the parameter is a register, push the register value on the stack
 * Otherwise it is on the stack already, and we need to push it again
 * 
 * @param nasm 
 * @param node 
 * @param symtable 
 * @param func 
 */
static void _CodeWriter_loadFunctionParam(
    FILE* nasm, Node* node, const ProgramSymbolTable* symtable,
    const FunctionSymbolTable* func
) {
    const Symbol* symbol = SymbolTable_resolve_from_node(symtable, func, node);

    fprintf(
        nasm,
        "; Chargement de l'argument '%s' sur la tête de pile\n"
        "push QWORD [rbp %+d]\n",
        symbol->identifier,
        symbol->addr
    );
}

/**
 * @brief Compute the address of an array and save it in ``rdx`` register
 * 
 * @param nasm 
 * @param node Ident node of the array
 * @param symtable 
 * @param func 
 */
static void _CodeWriter_ComputeArrayAddress(FILE* nasm,
                                   Node* node,
                                   const ProgramSymbolTable* symtable,
                                   const FunctionSymbolTable* func) {
    const Symbol* symbol = SymbolTable_resolve_from_node(symtable, func, node);

    fprintf(
        nasm,
        "; Chargement de l'adresse du tableau '%s' dans rdx\n",
        symbol->identifier
    );
    // rdx = Array address
    if (symbol->is_static) {
        fprintf(
            nasm,
            "mov rdx, global_vars + %d\n",
            symbol->addr
        );
    } else if (symbol->is_param) {
        _CodeWriter_loadFunctionParam(nasm, node, symtable, func);
        fprintf(
            nasm,
            "pop rdx\n"
        );
    } else /* symbol is local */ {
        fprintf(
            nasm,
            "lea rdx, [rbp %+d]; Calcul de l'adresse de %s[0] dans la pile\n",
            symbol->addr,
            symbol->identifier
        );
    }
}

/**
 * @brief Load the address of an array on the stack, in order to
 * pass it as an argument to a function.
 * 
 * @param nasm 
 * @param node 
 * @param symtable 
 * @param func 
 */
static void _CodeWriter_LoadArrayAddress(FILE* nasm,
                                  Node* node,
                                  const ProgramSymbolTable* symtable,
                                  const FunctionSymbolTable* func) {
    const Symbol* symbol = SymbolTable_resolve_from_node(symtable, func, node);

    _CodeWriter_ComputeArrayAddress(nasm, node, symtable, func);

    fprintf(
        nasm,
        "; Chargement de l'adresse du tableau '%s' sur la tête de pile\n",
        symbol->identifier
    );

    fprintf(
        nasm,
        "push rdx\n\n"
    );
}

/**
 * @brief Compute the address of an indexed element of an array,
 * and push it on the stack.
 * Should be used before loading or writing an element of an array
 * 
 * @param nasm 
 * @param node ArrayLR node
 * @param symtable 
 * @param func 
 */
static void _CodeWriter_ComputeArrayElementAddress(FILE* nasm,
                                           Node* node,
                                           const ProgramSymbolTable* symtable,
                                           const FunctionSymbolTable* func) {
    assert(node->label == ArrayLR && node->firstChild != NULL);
    const Symbol* symbol = SymbolTable_resolve_from_node(symtable, func, node);
    
    fprintf(
        nasm,
        "; Calcul de l'expression d'indexation du tableau '%s'\n",
        symbol->identifier
    );
    
    TreeReader_Expr(symtable, node->firstChild, nasm, func);

    fprintf(
        nasm,
        "pop rax\n" // rax = Index
    );

    _CodeWriter_ComputeArrayAddress(nasm, node, symtable, func);

    fprintf(
        nasm,
        "; Calcul de l'adresse d'un élément du tableau (%s) '%s'\n",
        symbol->is_static ? "global" : symbol->is_param ? "paramétré" : "local",
        symbol->identifier
    );

    fprintf(
        nasm,
        // lea : Compute effective address, without dereferencing
        "lea rax, [rdx + rax * %d]; Calcul de l'adresse de l'élément indexé\n"
        "push rax\n",
        symbol->type_size
    );
}

/**
 * @brief Load an element of an indexed array on the stack
 * 
 * @param nasm 
 * @param node ArrayLR node
 * @param symtable 
 * @param func 
 */
static void _CodeWriter_LoadArray(FILE* nasm,
                          Node* node,
                          const ProgramSymbolTable* symtable,
                          const FunctionSymbolTable* func) {
    assert(node->label == ArrayLR);
    const Symbol* symbol = SymbolTable_resolve_from_node(symtable, func, node);

    _CodeWriter_ComputeArrayElementAddress(nasm, node, symtable, func);

    fprintf(
        nasm,
        "; Chargement d'un élément du tableau '%s' sur la tête de pile\n"
        "pop rax\n"
        "push QWORD [rax]\n\n",
        symbol->identifier
    );
}

/**
 * @brief Load a value on the stack
 * 
 * @param nasm 
 * @param node Ident node, but not an array
 * @param symtable 
 * @param func 
 */
static void _CodeWriter_LoadValue(FILE* nasm, Node* node,
                          const ProgramSymbolTable* symtable,
                          const FunctionSymbolTable* func) {

    assert(node->label == Ident && node->firstChild == NULL);
    const Symbol* symbol = SymbolTable_resolve_from_node(symtable, func, node);

    if (symbol->is_param) {
        _CodeWriter_loadFunctionParam(nasm, node, symtable, func);
    }
    else if (symbol->is_static) {
        fprintf(
            nasm,
            "; Chargement de la variable globale '%s' sur la tête de pile\n"
            "push QWORD [global_vars + %d]\n",
            symbol->identifier,
            symbol->addr);
    } else /* local */ {
        fprintf(
            nasm,
            "; Chargement de la variable locale '%s' sur la tête de pile\n"
            "push QWORD [rbp %+d]\n",
            symbol->identifier,
            symbol->addr);
    }
}

void CodeWriter_LoadVar(FILE* nasm,
                        Node* node,
                        const ProgramSymbolTable* symtable,
                        const FunctionSymbolTable* func) {
    const Symbol* symbol = SymbolTable_resolve_from_node(symtable, func, node);

    if (symbol->symbol_type == SYMBOL_FUNCTION) {
        CodeWriter_CallFunction(nasm, node, symtable, func, symbol);
    } else if (symbol->symbol_type == SYMBOL_VALUE) {
        _CodeWriter_LoadValue(nasm, node, symtable, func);
    } else if (symbol->symbol_type == SYMBOL_ARRAY) {
        // If the array is not indexed, we just need to load the array address
        if (node->firstChild == NULL) {
            _CodeWriter_LoadArrayAddress(nasm, node, symtable, func);
        }
        // Else the array is indexed, we need to load the indexed element
        else {
            _CodeWriter_LoadArray(nasm, node, symtable, func);
        }
    }
    else {
        assert(0 && "We shouldn't be there");
    }
}

/**
 * @brief Write a stacked value to a variable (local, global or parameter)
 * 
 * @param nasm 
 * @param node 
 * @param symtable 
 * @param func 
 */
static void CodeWriter_WriteValue(FILE* nasm, Node* node,
                             const ProgramSymbolTable* symtable,
                             const FunctionSymbolTable* func) {

    assert(node->label == Ident);
    const Symbol* symbol = SymbolTable_resolve_from_node(symtable, func, node);

    if (symbol->is_static) {
        fprintf(
            nasm,
            "; Assignation de la dernière valeur de la pile dans la variable globale '%s'\n"
            "pop rax\n"
            "mov [global_vars + %d], rax\n",
            symbol->identifier,
            symbol->addr);
    }
    else if (symbol->is_param) {
        fprintf(
            nasm,
            "; Assignation de la dernière valeur de la pile dans l'argument '%s'\n"
            "pop rax\n"
            "mov [rbp %+d], rax\n",
            symbol->identifier,
            symbol->addr);
    } else /* local */ {
        fprintf(
            nasm,
            "; Assignation de la dernière valeur de la pile dans la variable locale '%s'\n"
            "pop rax\n"
            "mov [rbp %+d], rax\n",
            symbol->identifier,
            symbol->addr);
    }
}

/**
 * @brief Write a stacked value to an indexed element of an array
 * 
 * @param nasm 
 * @param node 
 * @param symtable 
 * @param func 
 */
static void CodeWriter_WriteArray(FILE* nasm, Node* node,
                             const ProgramSymbolTable* symtable,
                             const FunctionSymbolTable* func) {
    assert(node->label == ArrayLR);
    const Symbol* symbol = SymbolTable_resolve_from_node(symtable, func, node);

    _CodeWriter_ComputeArrayElementAddress(nasm, node, symtable, func);

    fprintf(
        nasm,
        "; Assignation de la dernière valeur de la pile dans l'élément du tableau '%s'\n",
        symbol->identifier
    );

    fprintf(
        nasm,
        "pop rax\n"
        "pop qword [rax]\n\n"
    );
}

void CodeWriter_WriteVar(FILE* nasm, Node* node,
                         const ProgramSymbolTable* symtable,
                         const FunctionSymbolTable* func) {
    assert(node->label == Ident || node->label == ArrayLR);

    const Symbol* symbol = SymbolTable_resolve_from_node(symtable, func, node);

    if (symbol->symbol_type == SYMBOL_VALUE) {
        CodeWriter_WriteValue(nasm, node, symtable, func);
    }
    else if (symbol->symbol_type == SYMBOL_ARRAY) {
        CodeWriter_WriteArray(nasm, node, symtable, func);
    }
    else {
        assert(0 && "We shouldn't be there");
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
        "sub rsp, %ld\n",
        func->locals.next_addr,
        func->locals.next_addr);

    // Move parameters to the callee's stack frame
    int nb_params_to_save = MIN(FunctionSymbolTable_get_param_count(func), 6);

    for (int i = 0; i < nb_params_to_save; ++i) {
        const Symbol* param = FunctionSymbolTable_get_param(func, i);
        fprintf(
            nasm,
            "; Move parameter '%s' to the stack frame\n"
            "mov [rbp %+d], %s\n",
            param->identifier,
            param->addr,
            Register_to_str(Register_param_to_reg(i)));
    }
    fputc(
        '\n',
        nasm
    );
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
