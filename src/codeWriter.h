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

void CodeWriter_Init_File(FILE* nasm);

void CodeWriter_End_File(FILE* nasm);

int CodeWriter_Ope(FILE* nasm, const Node* node);

int CodeWriter_ConstantNumber(FILE* nasm, const Node* node);

int CodeWriter_ConstantCharacter(FILE* nasm, const Node* node);

int CodeWriter_LoadVar(FILE* nasm, Node* node,
                       const ProgramSymbolTable* symtable, char* func_name);

int CodeWriter_WriteVar(FILE* nasm, Node* node,
                        const ProgramSymbolTable* symtable, char* func_name);