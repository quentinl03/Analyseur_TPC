/**
 * @file analyseur.c
 * @author Laborde Quentin
 * @brief
 * @date 31-01-2024
 *
 */

#include <stdio.h>
#include <stdlib.h>

#include "parser.h"
#include "tpc_bison.h"
#include "tree.h"

#include "symboltable.h"

int main(int argc, char* argv[]) {
    extern FILE* yyin;
    Node* abr = NULL;
    Option opt;
    opt = parser(argc, argv);

    if (opt.path) {
        yyin = fopen(opt.path, "r");
        if (!yyin) {
            perror("fopen");
            fprintf(stderr, "End of execution.\n");
            return 1;
        }
    }

    Error err = parser_bison(yyin, &abr);
    fclose(yyin);

    if (err < 0) {
        return -err;
    }

    ProgramSymbolTable symtable;
    ProgramSymbolTable_from_Prog(&symtable, abr);

    if (opt.flag_symtabs) {
        ProgramSymbolTable_print(&symtable);
    }
    if (opt.flag_tree) {
        printTree(abr);
    }
    if (abr) {
        deleteTree(abr);
    }

    return EXIT_SUCCESS;
}
