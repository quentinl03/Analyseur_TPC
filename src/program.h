#ifndef PROGRAM_H
#define PROGRAM_H

#include "tree.h"
#include "symboltable.h"
#include "parser.h"
#include <stdio.h>

typedef struct {
    ProgramSymbolTable symtable;
    Node* abr;
    Option opt;
    FILE* file_in;
    FILE* file_out;
} Program;

#endif
