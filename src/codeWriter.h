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

#include "symboltable.h"
#include "tree.h"

/**
 * @brief Write the header of the nasm file,
 * including the BSS section and the extern declaration
 * of utils.asm functions.
 * 
 * @param nasm File to write to
 * @param globals Symbol table of the global variables
 */
void CodeWriter_Init_File(FILE* nasm, const SymbolTable* globals);

void CodeWriter_End_File(FILE* nasm);

int CodeWriter_Ope(FILE* nasm, const Node* node);

int CodeWriter_ConstantNumber(FILE* nasm, const Node* node);

int CodeWriter_ConstantCharacter(FILE* nasm, const Node* node);

int CodeWriter_LoadVar(FILE* nasm, Node* node,
                       const ProgramSymbolTable* symtable, const FunctionSymbolTable* func);

int CodeWriter_WriteVar(FILE* nasm, Node* node,
                        const ProgramSymbolTable* symtable, const FunctionSymbolTable* func);

/**
 * @brief Write the start of a stack frame.
 * 
 * @param nasm FILE* to write to
 * @param func Function symbol table of the function that is being
 * @return int 
 */
int CodeWriter_stackFrame_start(FILE* nasm, const FunctionSymbolTable* func);

/**
 * @brief Write the end of a stack frame.
 * 
 * @param nasm 
 * @param func 
 * @return int 
 */
int CodeWriter_stackFrame_end(FILE* nasm, const FunctionSymbolTable* func);