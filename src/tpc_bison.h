#ifndef TPC_BISON_H
#define TPC_BISON_H

#include <stdio.h>

#include "tree.h"
#include "error.h"

Error parser_bison(FILE* f, Node** abr);

#endif
