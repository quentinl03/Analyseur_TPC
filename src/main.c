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

#include "program.h"

Program PROGRAM = {0};

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
        yyin = (PROGRAM.file_in = fopen(PROGRAM.opt.path, "r"));
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

    PROGRAM.file_out = fopen(PROGRAM.opt.output, "w");
    if (!PROGRAM.file_out) {
        perror("fopen");
        return EXIT_CODE(ERR_FILE_OPEN);
    }

    TreeReader_Prog(&symtable, PROGRAM.abr, PROGRAM.file_out);

    return EXIT_SUCCESS;
}
