/**
 * @file treeReader.c
 * @author Laborde Quentin & Seban Nicolas
 * @brief
 * @date 27-03-2024
 *
 * @copyright Copyright (c) 2024
 *
 */

#include "treeReader.h"

#include <assert.h>
#include <stdio.h>

#include "codeWriter.h"
#include "symboltable.h"
#include "tree.h"

int TreeReader_Prog(ProgramSymbolTable* prog_st, Tree tree, FILE* nasm) {
    // Si est pas dans le noeux c'est grave car la suite du parcours est foutu.
    assert(tree->label == Prog);

    TreeReader_DeclFoncts(prog_st, SECONDCHILD(tree), nasm);

    return 0;
}

int TreeReader_DeclFoncts(ProgramSymbolTable* prog_st, Tree tree, FILE* nasm) {
    // Si est pas dans le noeux c'est grave car la suite du parcours est foutu.
    assert(tree->label == DeclFoncts);

    // On parcours les noeux DeclFonct
    for (Node* child = tree->firstChild;
         child != NULL;
         child = child->nextSibling) {
        // On passe de DeclFonct à SuiteInstr

        TreeReader_SuiteInst(prog_st, SECONDCHILD(SECONDCHILD(child)),
                             SECONDCHILD(FIRSTCHILD(child))->att.ident, nasm);
    }

    return 0;
}

int TreeReader_SuiteInst(ProgramSymbolTable* prog_st, Tree tree,
                         char* func_name, FILE* nasm) {
    printf("SuiteInst\n");
    printf("func_name: %s\n", func_name);

    return 0;
}