/**
 * @file analyseur.c
 * @author Laborde Quentin
 * @brief
 * @date 31-01-2024
 *
 */

#include <stdio.h>
#include <stdlib.h>

#include "codeWriter.h"
#include "parser.h"
#include "semantic.h"
#include "symboltable.h"
#include "tpc_bison.h"
#include "tree.h"
#include "treeReader.h"

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
            return EXIT_CODE(ERR_FILE_OPEN);
        }
    }

    ErrorType err = parser_bison(yyin, &abr);
    fclose(yyin);

    if (IS_PARSE_ERROR(err)) {
        return EXIT_CODE(err);
        ;
    }

    if (opt.flag_show_tree) {
        printTree(abr);
    }

    if (opt.flag_only_tree) {
        if (abr) {
            deleteTree(abr);
        }
        return EXIT_SUCCESS;
    }

    ProgramSymbolTable symtable;
    err = ProgramSymbolTable_from_Prog(&symtable, abr);
    if (opt.flag_symtabs) {
        ProgramSymbolTable_print(&symtable);
    }
    err |= Semantic_check(abr, &symtable);
    if (IS_SEMANTIC(err) || IS_CRITICAL(err)) {
        return EXIT_CODE(err);
    }

    FILE* file = fopen(opt.output, "w");
    if (!file) {
        perror("fopen");
        return EXIT_CODE(ERR_FILE_OPEN);
    }

    TreeReader_Prog(&symtable, abr, file);

    if (abr) {
        deleteTree(abr);
    }

    return EXIT_SUCCESS;
}
