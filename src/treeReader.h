/**
 * @file treeReader.h
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

int TreeReader_Prog(ProgramSymbolTable* table, Tree tree, FILE* nasm);

int TreeReader_DeclFoncts(ProgramSymbolTable* table, Tree tree, FILE* nasm);

int TreeReader_SuiteInst(ProgramSymbolTable* table, Tree tree, char* func_name, FILE* nasm);