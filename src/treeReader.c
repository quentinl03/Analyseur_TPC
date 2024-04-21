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

#include "error.h"

// ! à retirer avant rendu debug parcours arbre laisser pour le moment
// static const char* NODE_STRING[] = {
//     FOREACH_NODE(GENERATE_STRING)};

static ErrorType _Instr_Return(const ProgramSymbolTable* table, Tree tree, FILE* nasm, const FunctionSymbolTable* func);
static ErrorType _Instr_Assignation(const ProgramSymbolTable* table, Tree tree, FILE* nasm, const FunctionSymbolTable* func);
static ErrorType _TreeReader_DeclFoncts(const ProgramSymbolTable* table, Tree tree, FILE* nasm);
static ErrorType TreeReader_SuiteInst(
    const ProgramSymbolTable* table, Tree tree,
    const FunctionSymbolTable* func, FILE* nasm
);

ErrorType TreeReader_Prog(const ProgramSymbolTable* table, Tree tree, FILE* nasm) {
    // Si est pas dans le noeux c'est grave car la suite du parcours est foutu.
    assert(tree->label == Prog);

    CodeWriter_Init_File(nasm, &table->globals);
    return _TreeReader_DeclFoncts(table, SECONDCHILD(tree), nasm);
}

static ErrorType _TreeReader_Corps(
    const ProgramSymbolTable* prog,
    const FunctionSymbolTable* func,
    Tree tree, FILE* nasm
) {
    assert(tree->label == Corps);
    ErrorType err = ERR_NONE;
    // Implement stack frame
    CodeWriter_stackFrame_start(nasm, func);
    err |= TreeReader_SuiteInst(prog, SECONDCHILD(tree), func, nasm);
    CodeWriter_stackFrame_end(nasm, func);
    CodeWriter_Return(nasm);

    return err;
}

/**
 * @brief Generate code for a function.
 * Writes function's label and its body (Corps).
 * 
 * @param prog
 * @param tree Tree DeclFonct node 
 * @param nasm 
 * @return int 
 */
static ErrorType _TreeReader_DeclFonct(
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
    return _TreeReader_Corps(prog, func, SECONDCHILD(tree), nasm);
}


static ErrorType _TreeReader_DeclFoncts(const ProgramSymbolTable* table, Tree tree, FILE* nasm) {
    // Si est pas dans le noeux c'est grave car la suite du parcours est foutu.
    assert(tree->label == DeclFoncts);
    ErrorType err = ERR_NONE;

    // On parcourt les noeux DeclFonct
    for (Node* child = tree->firstChild;
         child != NULL;
         child = child->nextSibling
    ) {
        err |= _TreeReader_DeclFonct(table, child, nasm);
    }

    return ERR_NONE;
}

static ErrorType TreeReader_SuiteInst(const ProgramSymbolTable* table, Tree tree,
                         const FunctionSymbolTable* func, FILE* nasm) {
    // printf("SuiteInst \t");
    // printf("func_name: %s\n", func_name);
    ErrorType err = ERR_NONE;

    for (Node* child = tree->firstChild; child != NULL; child = child->nextSibling) {
        switch (child->label) {
            case Return: // TODO : Attentions aux returns surnuméraires !
                err |= _Instr_Return(table, child, nasm, func);
                break;
            case Addsub:
            case Divstar:
            case Character:
            case Num:
            case Ident:
                // TODO ajouter les autre case d'expression
                err |= TreeReader_Expr(table, child, nasm, func);
                break;
            case Assignation:
                // TODO WIP
                err |= _Instr_Assignation(table, child, nasm, func);
                break;
            default:
                break;
        }
        // printf("child->label: %s\n", NODE_STRING[child->label]);
    }

    return ERR_NONE;
}

/******************/
/* Instr Unitaire */
/******************/

ErrorType TreeReader_Expr(const ProgramSymbolTable* table, Tree tree, FILE* nasm, const FunctionSymbolTable* func) {
    // printf("TreeReader_Expr\n");
    ErrorType err = ERR_NONE;
    switch (tree->label) {
        case Addsub:
        case Divstar:
            // printf("Addsub or divstar\n");
            TreeReader_Expr(table, FIRSTCHILD(tree), nasm, func);
            TreeReader_Expr(table, SECONDCHILD(tree), nasm, func);
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
    return err;
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
static ErrorType _Instr_Return(const ProgramSymbolTable* table, Tree tree, FILE* nasm, const FunctionSymbolTable* func) {
    // printf("Instr_Return\n");
    ErrorType err = ERR_NONE;
    const Symbol* sym = SymbolTable_get(&table->globals, func->identifier);

    if (sym->type != type_void) /* Non void */ {
        if (FIRSTCHILD(tree)) {
            err |= TreeReader_Expr(table, FIRSTCHILD(tree), nasm, func);
            CodeWriter_Return_Expr(nasm);
        }
        else {
            CodeError_print(
                (CodeError) {
                    .err = WARN_RETURN_WITHOUT_VALUE,
                    .line = tree->lineno,
                    .column = 0,
                },
                "'return' with no value, in function returning non-void"
            );
        }
    }
    return err;
}

static ErrorType _Instr_Assignation(const ProgramSymbolTable* table, Tree tree, FILE* nasm, const FunctionSymbolTable* func) {
    // TODO : Verif si la fonction marche
    // printf("Instr_Assignation\n");
    ErrorType err = ERR_NONE;
    err |= TreeReader_Expr(table, SECONDCHILD(tree), nasm, func);
    CodeWriter_WriteVar(nasm, FIRSTCHILD(tree), table, func);
    return err;
}
