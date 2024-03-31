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

// ! à retirer avant rendu debug parcours arbre laisser pour le moment
// static const char* NODE_STRING[] = {
//     FOREACH_NODE(GENERATE_STRING)};

static int _Instr_Expr(ProgramSymbolTable* table, Tree tree, FILE* nasm, char* func_name);
static int _Instr_Return(ProgramSymbolTable* table, Tree tree, FILE* nasm, char* func_name);
static int _Instr_Assignation(ProgramSymbolTable* table, Tree tree, FILE* nasm, char* func_name);

int TreeReader_Prog(ProgramSymbolTable* table, Tree tree, FILE* nasm) {
    // Si est pas dans le noeux c'est grave car la suite du parcours est foutu.
    assert(tree->label == Prog);

    TreeReader_DeclFoncts(table, SECONDCHILD(tree), nasm);

    return 0;
}

int TreeReader_DeclFoncts(ProgramSymbolTable* table, Tree tree, FILE* nasm) {
    // Si est pas dans le noeux c'est grave car la suite du parcours est foutu.
    assert(tree->label == DeclFoncts);

    // On parcours les noeux DeclFonct
    for (Node* child = tree->firstChild;
         child != NULL;
         child = child->nextSibling) {
        // On passe de DeclFonct à SuiteInstr
        TreeReader_SuiteInst(table, SECONDCHILD(SECONDCHILD(child)),
                             SECONDCHILD(FIRSTCHILD(child))->att.ident, nasm);
    }

    return 0;
}

int TreeReader_SuiteInst(ProgramSymbolTable* table, Tree tree,
                         char* func_name, FILE* nasm) {
    // printf("SuiteInst \t");
    // printf("func_name: %s\n", func_name);

    for (Node* child = tree->firstChild; child != NULL; child = child->nextSibling) {
        switch (child->label) {
            case Return:
                _Instr_Return(table, child, nasm, func_name);
                break;
            case Addsub:
            case Divstar:
            case Character:
            case Num:
            case Ident:
                // TODO ajouter les autre case d'expression
                _Instr_Expr(table, child, nasm, func_name);
                break;
            case Assignation:
                // TODO WIP
                _Instr_Assignation(table, child, nasm, func_name);
                break;
            default:
                break;
        }
        // printf("child->label: %s\n", NODE_STRING[child->label]);
    }

    return 0;
}

/******************/
/* Instr Unitaire */
/******************/

static int _Instr_Expr(ProgramSymbolTable* table, Tree tree, FILE* nasm, char* func_name) {
    // printf("Instr_Expr\n");
    switch (tree->label) {
        case Addsub:
        case Divstar:
            // printf("Addsub or divstar\n");
            _Instr_Expr(table, FIRSTCHILD(tree), nasm, func_name);
            _Instr_Expr(table, SECONDCHILD(tree), nasm, func_name);
            CodeWriter_Ope(nasm, tree);
            break;
        case Ident:
            // printf("Ident\n");
            CodeWriter_LoadVar(nasm, tree, table, func_name);
            break;
        case Num:
            // printf("Num\n");
            CodeWriter_ConstantNumber(nasm, tree);
            break;
        case Character:
            // printf("Char\n");
            CodeWriter_ConstantCharacter(nasm, tree);
            break;
        default:
            // ! Noeud non géré voloraiement ou non
            assert(0 && "We shoudn't be there");
            break;
    }
    return 0;
}

static int _Instr_Return(ProgramSymbolTable* table, Tree tree, FILE* nasm, char* func_name) {
    // printf("Instr_Return\n");
    _Instr_Expr(table, FIRSTCHILD(tree), nasm, func_name);
    return 0;
}

static int _Instr_Assignation(ProgramSymbolTable* table, Tree tree, FILE* nasm, char* func_name) {
    // TODO : Verif si la fucntion marche
    // printf("Instr_Assignation\n");
    _Instr_Expr(table, SECONDCHILD(tree), nasm, func_name);
    CodeWriter_WriteVar(nasm, FIRSTCHILD(tree), table, func_name);
    return 0;
}