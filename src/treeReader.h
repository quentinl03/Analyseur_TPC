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

/**
 * @brief Traverse the tree and write the nasm code
 * 
 * @param table pre-generated Program Symbol table
 * @param tree Bison's generated tree must be a `Program` node
 * @param nasm Output file
 */
void TreeReader_Prog(const ProgramST* table,
                     Tree tree, FILE* nasm);

/**
 * @brief Generate nasm code to evaluate an expression and
 * push the result on the stack
 * 
 * @param table Program's symbol table
 * @param tree Any expression node
 * @param nasm Output file
 * @param func Caller function's symbol table
 */
void TreeReader_Expr(const ProgramST* table,
                     Tree tree, FILE* nasm,
                     const FunctionST* func);
