#ifndef TPC_BISON_H
#define TPC_BISON_H

#include <stdio.h>

#include "tree.h"
#include "error.h"

ErrorType parser_bison(FILE* f, Node** abr);

#endif
