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

static int _Instr_Expr(const ProgramSymbolTable* table, Tree tree, FILE* nasm, const FunctionSymbolTable* func);
static int _Instr_Return(const ProgramSymbolTable* table, Tree tree, FILE* nasm, const FunctionSymbolTable* func);
static int _Instr_Assignation(const ProgramSymbolTable* table, Tree tree, FILE* nasm, const FunctionSymbolTable* func);

int TreeReader_Prog(const ProgramSymbolTable* table, Tree tree, FILE* nasm) {
    // Si est pas dans le noeux c'est grave car la suite du parcours est foutu.
    assert(tree->label == Prog);

    TreeReader_DeclFoncts(table, SECONDCHILD(tree), nasm);

    return 0;
}

static int _TreeReader_Corps(
    const ProgramSymbolTable* prog,
    const FunctionSymbolTable* func,
    Tree tree, FILE* nasm
) {
    assert(tree->label == Corps);
    // Implement stack frame
    // CodeWriter_stackFrame_start(func, nasm);
    TreeReader_SuiteInst(prog, SECONDCHILD(tree), func, nasm);
    // CodeWriter_stackFrame_end(table, nasm);
    return 0;
}


int TreeReader_DeclFoncts(const ProgramSymbolTable* table, Tree tree, FILE* nasm) {
    // Si est pas dans le noeux c'est grave car la suite du parcours est foutu.
    assert(tree->label == DeclFoncts);

    // On parcours les noeux DeclFonct
    for (Node* child = tree->firstChild;
         child != NULL;
         child = child->nextSibling) {
        // On passe de DeclFonct à SuiteInstr
        FunctionSymbolTable* func = SymbolTable_get_from_func_name(table, FIRSTCHILD(child)->firstChild->nextSibling->att.ident);
        _TreeReader_Corps(table, func, SECONDCHILD(child), nasm);
    }

    return 0;
}

int TreeReader_SuiteInst(const ProgramSymbolTable* table, Tree tree,
                         const FunctionSymbolTable* func, FILE* nasm) {
    // printf("SuiteInst \t");
    // printf("func_name: %s\n", func_name);

    for (Node* child = tree->firstChild; child != NULL; child = child->nextSibling) {
        switch (child->label) {
            case Return:
                _Instr_Return(table, child, nasm, func);
                break;
            case Addsub:
            case Divstar:
            case Character:
            case Num:
            case Ident:
                // TODO ajouter les autre case d'expression
                _Instr_Expr(table, child, nasm, func);
                break;
            case Assignation:
                // TODO WIP
                _Instr_Assignation(table, child, nasm, func);
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

static int _Instr_Expr(const ProgramSymbolTable* table, Tree tree, FILE* nasm, const FunctionSymbolTable* func) {
    // printf("Instr_Expr\n");
    switch (tree->label) {
        case Addsub:
        case Divstar:
            // printf("Addsub or divstar\n");
            _Instr_Expr(table, FIRSTCHILD(tree), nasm, func);
            _Instr_Expr(table, SECONDCHILD(tree), nasm, func);
            CodeWriter_Ope(nasm, tree);
            break;
        case Ident:
            // printf("Ident\n");
            CodeWriter_LoadVar(nasm, tree, table, func);
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

static int _Instr_Return(const ProgramSymbolTable* table, Tree tree, FILE* nasm, const FunctionSymbolTable* func) {
    // printf("Instr_Return\n");
    _Instr_Expr(table, FIRSTCHILD(tree), nasm, func);
    return 0;
}

static int _Instr_Assignation(const ProgramSymbolTable* table, Tree tree, FILE* nasm, const FunctionSymbolTable* func) {
    // TODO : Verif si la fucntion marche
    // printf("Instr_Assignation\n");
    _Instr_Expr(table, SECONDCHILD(tree), nasm, func);
    CodeWriter_WriteVar(nasm, FIRSTCHILD(tree), table, func);
    return 0;
}
