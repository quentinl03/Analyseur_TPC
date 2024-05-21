#ifndef PROGRAM_H
#define PROGRAM_H

#include <stdio.h>

#include "parser.h"
#include "symbolTable.h"
#include "tree.h"

typedef struct {
    ProgramST symtable;
    Node* abr;
    Option opt;
    FILE* file_in;
    FILE* file_out;
} Program;

#endif
