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
    Node* abr;
    Option opt;
    opt = parser(argc, argv);
    if (opt.flag_help) {
        return 0;
    }

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

    SymbolTable global_vars;
    SymbolTable_init(&global_vars);
    SymbolTable_from_DeclVar(&global_vars, abr->firstChild);

    if (opt.flag_tree) {
        printTree(abr);
        SymbolTable_print(&global_vars);
    }
    if (abr) {
        deleteTree(abr);
    }

    return EXIT_SUCCESS;
}
