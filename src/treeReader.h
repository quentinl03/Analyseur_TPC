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
#include "error.h"

ErrorType TreeReader_Prog(const ProgramSymbolTable* table, Tree tree, FILE* nasm);
