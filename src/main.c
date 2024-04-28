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

typedef struct {
    ProgramSymbolTable symtable;
    Node* abr;
    Option opt;
    FILE* file_out;
} Program;

Program PROGRAM;

void atexit_function(void) {
    if (PROGRAM.abr) {
        deleteTree(PROGRAM.abr);
    }
    if (PROGRAM.file_out &&
        PROGRAM.file_out != stdout &&
        PROGRAM.file_out != stderr) {
        fclose(PROGRAM.file_out);
    }
    //! @todo free symtable
}

int main(int argc, char* argv[]) {
    atexit(atexit_function);
    extern FILE* yyin;
    PROGRAM.opt = parser(argc, argv);

    if (PROGRAM.opt.path) {
        yyin = fopen(PROGRAM.opt.path, "r");
        if (!yyin) {
            perror("fopen");
            fprintf(stderr, "End of execution.\n");
            return EXIT_CODE(ERR_FILE_OPEN);
        }
    }

    ErrorType err = parser_bison(yyin, &PROGRAM.abr);
    // fclose(yyin);

    if (IS_PARSE_ERROR(err)) {
        return EXIT_CODE(err);
    }

    if (PROGRAM.opt.flag_show_tree) {
        printTree(PROGRAM.abr);
    }

    if (PROGRAM.opt.flag_only_tree) {
        return EXIT_SUCCESS;
    }

    ProgramSymbolTable symtable;
    err = ProgramSymbolTable_from_Prog(&symtable, PROGRAM.abr);
    if (PROGRAM.opt.flag_symtabs) {
        ProgramSymbolTable_print(&symtable);
    }
    if (IS_SEMANTIC(err) || IS_CRITICAL(err)) {
        return EXIT_CODE(err);
    }

    err |= Semantic_check(PROGRAM.abr, &symtable);
    if (PROGRAM.opt.flag_semantic || IS_SEMANTIC(err) || IS_CRITICAL(err)) {
        return EXIT_CODE(err);
    }

    FILE* file = fopen(PROGRAM.opt.output, "w");
    if (!file) {
        perror("fopen");
        return EXIT_CODE(ERR_FILE_OPEN);
    }

    TreeReader_Prog(&symtable, PROGRAM.abr, file);

    return EXIT_SUCCESS;
}
