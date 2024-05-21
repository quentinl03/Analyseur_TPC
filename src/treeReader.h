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

#include "symbolTable.h"
#include "tree.h"

void TreeReader_Prog(const ProgramST* table,
                     Tree tree, FILE* nasm);

void TreeReader_Expr(const ProgramST* table,
                     Tree tree, FILE* nasm,
                     const FunctionST* func);
