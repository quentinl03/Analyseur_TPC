/**
 * @file CodeWriter.h
 * @author Laborde Quentin & Seban Nicolas
 * @brief
 * @date 27-03-2024
 *
 * @copyright Copyright (c) 2024
 *
 */

#include <stdio.h>
#define PATH_BUILTINS "./src/builtins.asm"

#include "symbolTable.h"
#include "tree.h"

/**
 * @brief Write the header of the nasm file,
 * including the BSS section and the extern declaration
 * of utils.asm functions.
 *
 * @param nasm File to write into
 * @param globals Symbol table of the global variables
 */
void CodeWriter_Init_File(FILE* nasm, const SymbolTable* globals);

/**
 * @brief
 *
 * @param nasm
 */
void CodeWriter_load_builtins(FILE* nasm);

/**
 * @brief Write code to call a function with its arguments
 *
 * @param nasm File to write to
 * @param node Function node (Ident node with EmptyArgs or ListExp node)
 * @param symtable
 * @param caller
 */
void CodeWriter_CallFunction(
    FILE* nasm,
    Node* node,
    const ProgramST* symtable,
    const FunctionST* caller);

/**
 * @brief Call a function, and push the result on the stack.
 * Called function should be non-void.
 *
 * @param nasm
 * @param node
 * @param symtable
 * @param caller
 */
void CodeWriter_CallFunctionAsExpression(
    FILE* nasm,
    Node* callee_node,
    const ProgramST* symtable,
    const FunctionST* caller);

/**
 * @brief Evaluate an arithmetic binary operation
 * Childs of the function should be evaluated before calling this function :
 * Pop two values from the stack, apply the operation and push the result.
 *
 * @param nasm
 * @param node
 */
void CodeWriter_Ope_Arith(FILE* nasm, const Node* node);

/**
 * @brief Write a unary operation to the nasm file.
 *
 * @param nasm File to write into
 * @param node Node to write
 */
void CodeWriter_Ope_Unaire(FILE* nasm, const Node* node);

/**
 * @brief Write a constant number to the nasm file.
 * Push the constant value to the stack.
 *
 * @param nasm File to write into
 * @param node Node to write
 */
void CodeWriter_ConstantNumber(FILE* nasm, const Node* node);

/**
 * @brief Write a constant character to the nasm file.
 *
 * @param nasm File to write into
 * @param node Node to write
 */
void CodeWriter_ConstantCharacter(FILE* nasm, const Node* node);

/**
 * @brief Push the variable value to the stack.
 * If the variable is a global variable, use bss section.
 * If the variable is a local variable, use the stack.
 * If the variable is a parameter, use registers.
 *
 * @param nasm File to write into
 * @param node Node to write
 * @param symtable Program symbol table
 * @param func Function symbol table
 */
void CodeWriter_LoadVar(FILE* nasm, Node* node,
                        const ProgramST* symtable,
                        const FunctionST* func);

/**
 * @brief Change the value of a variable from the last value on the stack.
 * If the variable is a global variable, use bss section.
 * If the variable is a local variable, use the stack.
 * If the variable is a parameter, use registers.
 *
 * @param nasm File to write into
 * @param node Node to write
 * @param symtable Program symbol table
 * @param func Function symbol table
 */
void CodeWriter_WriteVar(FILE* nasm, Node* node,
                         const ProgramST* symtable,
                         const FunctionST* func);

/**
 * @brief Write the start of a stack frame.
 *
 * @param nasm File to write into
 * @param func Function symbol table
 */
void CodeWriter_stackFrame_start(FILE* nasm, const FunctionST* func);

/**
 * @brief Write the end of a stack frame.
 *
 * @param nasm File to write into
 * @param func Function symbol table
 */
void CodeWriter_stackFrame_end(FILE* nasm, const FunctionST* func);

/**
 * @brief Write the label of a function.
 *
 * @param nasm File to write into
 * @param func Function symbol table
 */
void CodeWriter_FunctionLabel(FILE* nasm, const FunctionST* func);

/**
 * @brief Write `ret` instruction.
 *
 * @param nasm File to write into
 */
void CodeWriter_Return(FILE* nasm);

/**
 * @brief Move computed expression present on stack's head
 * to `rax` register
 *
 * @param nasm File to write into
 */
void CodeWriter_Return_Expr(FILE* nasm);

void CodeWriter_Cmp(FILE* nasm, Node* Node, int cmp_number);

void CodeWriter_If_Init(FILE* nasm, int if_number);
void CodeWriter_If_Else(FILE* nasm, int if_number);
void CodeWriter_If_End(FILE* nasm, int if_number);

void CodeWriter_While_Init(FILE* nasm, int while_number);
void CodeWriter_While_Eval(FILE* nasm, int while_number);
void CodeWriter_While_End(FILE* nasm, int while_number);

/**
 * @brief Write code for a boolean operation
 * Childs of the node shouldn't be already evaluated, as they
 * will be evaluated in this function.
 *
 * @param nasm
 * @param node
 * @param prog
 * @param func
 */
void CodeWriter_Ope_Bool(FILE* nasm,
                         const Tree node,
                         const ProgramST* prog,
                         const FunctionST* func);

/**
 * @brief Logical not operation.
 * Expects the value to negate to be on the top of the stack.
 *
 * @param nasm
 */
void CodeWriter_Ope_Bool_Not(FILE* nasm);
