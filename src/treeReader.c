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

    CodeWriter_Init_File(nasm, &table->globals);
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
    CodeWriter_stackFrame_start(nasm, func);
    TreeReader_SuiteInst(prog, SECONDCHILD(tree), func, nasm);
    CodeWriter_stackFrame_end(nasm, func);
    CodeWriter_Return(nasm);
    return 0;
}

/**
 * @brief Generate code for a function.
 * Writes function's label, and its body (Corps).
 * 
 * @param prog
 * @param tree Tree DeclFonct node 
 * @param nasm 
 * @return int 
 */
static int _TreeReader_DeclFonct(
    const ProgramSymbolTable* prog,
    Tree tree, FILE* nasm
) {
    assert(tree->label == DeclFonct);
    FunctionSymbolTable* func = SymbolTable_get_from_func_name(
        prog,
        // On passe de DeclFonct à SuiteInstr
        FIRSTCHILD(tree)->firstChild->nextSibling->att.ident
    );
    CodeWriter_FunctionLabel(nasm, func);
    _TreeReader_Corps(prog, func, SECONDCHILD(tree), nasm);

    return 0;
}


int TreeReader_DeclFoncts(const ProgramSymbolTable* table, Tree tree, FILE* nasm) {
    // Si est pas dans le noeux c'est grave car la suite du parcours est foutu.
    assert(tree->label == DeclFoncts);

    // On parcourt les noeux DeclFonct
    for (Node* child = tree->firstChild;
         child != NULL;
         child = child->nextSibling
    ) {
        _TreeReader_DeclFonct(table, child, nasm);
    }

    return 0;
}

int TreeReader_SuiteInst(const ProgramSymbolTable* table, Tree tree,
                         const FunctionSymbolTable* func, FILE* nasm) {
    // printf("SuiteInst \t");
    // printf("func_name: %s\n", func_name);

    for (Node* child = tree->firstChild; child != NULL; child = child->nextSibling) {
        switch (child->label) {
            case Return: // TODO : Attentions aux returns surnuméraires !
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

/**
 * @brief If the function is non-void (returns a value) move computed
 * expression to rax register (result of the expression).
 * If the function returns void, do nothing.
 * 
 * @param table 
 * @param tree 
 * @param nasm 
 * @param func 
 * @return int 
 */
static int _Instr_Return(const ProgramSymbolTable* table, Tree tree, FILE* nasm, const FunctionSymbolTable* func) {
    // printf("Instr_Return\n");
    const Symbol* sym = SymbolTable_get(&table->globals, func->identifier);
    if (sym->type != type_void) /* Non void */ {
        _Instr_Expr(table, FIRSTCHILD(tree), nasm, func);
        // TODO : Write code to pop stack's value to rax
    }
    return 0;
}

static int _Instr_Assignation(const ProgramSymbolTable* table, Tree tree, FILE* nasm, const FunctionSymbolTable* func) {
    // TODO : Verif si la fonction marche
    // printf("Instr_Assignation\n");
    _Instr_Expr(table, SECONDCHILD(tree), nasm, func);
    CodeWriter_WriteVar(nasm, FIRSTCHILD(tree), table, func);
    return 0;
}
