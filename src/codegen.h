#ifndef CODEGEN_H
#define CODEGEN_H

#include "tree.h"
#include "symboltable.h"
#include <stdio.h>


/**
 * @brief Emit assembly code
 * 
 * @param output File output to write ASM code to
 * @param tree A tree
 * @param symbolTable A symbol table to look up variables
 * @return int 
 */
int CodeGen_parse(const char* output, const Tree tree, ProgramSymbolTable* symbolTable);


#endif
