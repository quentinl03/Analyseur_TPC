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

int GLOBAL_CMP;

// ! à retirer avant rendu debug parcours arbre laisser pour le moment
static const char* NODE_STRING[] = {
    FOREACH_NODE(GENERATE_STRING)};

static void _Instr_Return(const ProgramSymbolTable* table,
                          Tree tree, FILE* nasm,
                          const FunctionSymbolTable* func);
static void _Instr_Assignation(const ProgramSymbolTable* table,
                               Tree tree, FILE* nasm,
                               const FunctionSymbolTable* func);
static void _TreeReader_DeclFoncts(const ProgramSymbolTable* table,
                                   Tree tree, FILE* nasm);
static void TreeReader_SuiteInst(const ProgramSymbolTable* table, Tree tree,
                                 const FunctionSymbolTable* func, FILE* nasm);

static void _Instr_If(const ProgramSymbolTable* table,
                      Tree tree, FILE* nasm,
                      const FunctionSymbolTable* func);

static void _Instr_While(const ProgramSymbolTable* table,
                         Tree tree, FILE* nasm,
                         const FunctionSymbolTable* func);

/**
 * @brief Write code for a function body.
 *
 * @param table
 * @param tree SuitInstr node, or a single instruction node
 * @param func
 * @param nasm
 */
static void TreeReader_SuiteInst(const ProgramSymbolTable* table, Tree tree,
                                 const FunctionSymbolTable* func, FILE* nasm) {

    assert(
        tree->label == SuiteInstr || tree->label == Return
        || tree->label == Assignation || tree->label == Ident
        || tree->label == If || tree->label == While || tree->label == EmptyInstr
    );

    Node* child = tree->label == SuiteInstr ? FIRSTCHILD(tree) : tree;

    for (; child != NULL; child = child->nextSibling) {
        switch (child->label) {
            case Return:
                _Instr_Return(table, child, nasm, func);
                break;
            case Ident:
                CodeWriter_CallFunction(nasm, child, table, func);
                break;
            case Assignation:
                _Instr_Assignation(table, child, nasm, func);
                break;
            case If:
                _Instr_If(table, child, nasm, func);
                break;
            case While:
                _Instr_While(table, child, nasm, func);
                break;
            case SuiteInstr:
                TreeReader_SuiteInst(table, child, func, nasm);
                break;
            case EmptyInstr:
                break;
            default:
                fprintf(stderr, "Node not managed: %s\n", NODE_STRING[child->label]);
                assert(0 && "We shoudn't be there\n");
        }
        // We stop the loop if the we are not in a SuiteInstr node
        if (tree->label != SuiteInstr) {
            break;
        }
    }
}
/**
 * @brief
 *
 * @param prog
 * @param func
 * @param tree
 * @param nasm
 */
static void _TreeReader_Corps(const ProgramSymbolTable* prog,
                              const FunctionSymbolTable* func,
                              Tree tree, FILE* nasm) {
    assert(tree->label == Corps);
    // Implement stack frame
    CodeWriter_stackFrame_start(nasm, func);
    TreeReader_SuiteInst(prog, SECONDCHILD(tree), func, nasm);
    CodeWriter_stackFrame_end(nasm, func);
    CodeWriter_Return(nasm);
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
static void _TreeReader_DeclFonct(const ProgramSymbolTable* prog,
                                  Tree tree, FILE* nasm) {
    assert(tree->label == DeclFonct);
    FunctionSymbolTable* func = FunctionSymbolTable_get_from_name(
        prog,
        // DeclFonct->EnTeteFonct->Ident
        FIRSTCHILD(tree)->firstChild->nextSibling->att.ident);
    CodeWriter_FunctionLabel(nasm, func);
    _TreeReader_Corps(prog, func, SECONDCHILD(tree), nasm);
}

static void _TreeReader_DeclFoncts(const ProgramSymbolTable* table,
                                   Tree tree, FILE* nasm) {
    // Si est pas dans le noeux c'est grave car la suite du parcours est foutu.
    assert(tree->label == DeclFoncts);
    // On parcourt les noeux DeclFonct
    for (Node* child = tree->firstChild;
         child != NULL;
         child = child->nextSibling) {
        _TreeReader_DeclFonct(table, child, nasm);
    }
}

void TreeReader_Prog(const ProgramSymbolTable* table, Tree tree, FILE* nasm) {
    // Si est pas dans le noeux c'est grave car la suite du parcours est foutu.
    assert(tree->label == Prog);

    CodeWriter_Init_File(nasm, &table->globals);
    _TreeReader_DeclFoncts(table, SECONDCHILD(tree), nasm);
    CodeWriter_load_builtins(nasm);
}

/******************/
/* Instr Unitaire */
/******************/

void TreeReader_Expr(const ProgramSymbolTable* table,
                     Tree tree, FILE* nasm,
                     const FunctionSymbolTable* func) {
    switch (tree->label) {
        case AddsubU:
            TreeReader_Expr(table, FIRSTCHILD(tree), nasm, func);
            CodeWriter_Ope_Unaire(nasm, tree);
            break;
        case Or:
        case And:
            CodeWriter_Ope_Bool(nasm, tree, table, func);
            break;
        case Not:
            TreeReader_Expr(table, FIRSTCHILD(tree), nasm, func);
            CodeWriter_Ope_Bool_Not(nasm);
            break;
        case Addsub:
        case Divstar:
            TreeReader_Expr(table, FIRSTCHILD(tree), nasm, func);
            TreeReader_Expr(table, SECONDCHILD(tree), nasm, func);
            CodeWriter_Ope_Arith(nasm, tree);
            break;
        case Ident:
        case ArrayLR:
            CodeWriter_LoadVar(nasm, tree, table, func);
            break;
        case Num:
            CodeWriter_ConstantNumber(nasm, tree);
            break;
        case Character:
            CodeWriter_ConstantCharacter(nasm, tree);
            break;
        case Eq:
        case Order:
            TreeReader_Expr(table, FIRSTCHILD(tree), nasm, func);
            TreeReader_Expr(table, SECONDCHILD(tree), nasm, func);
            CodeWriter_Cmp(nasm, tree, GLOBAL_CMP++);
            break;
        default:
            // ! Noeud non géré voloraiement ou non
            fprintf(stderr, "Node not managed: %s\n", NODE_STRING[tree->label]);
            assert(0 && "We shoudn't be there\n");
    }
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
static void _Instr_Return(const ProgramSymbolTable* table,
                          Tree tree, FILE* nasm,
                          const FunctionSymbolTable* func) {
    // printf("Instr_Return\n");
    const Symbol* sym = SymbolTable_get(&table->globals, func->identifier);

    if (sym->type != type_void) /* Non void */ {
        // Verifiy if a value to returns exists, see _Instr_Return
        if (FIRSTCHILD(tree)) {
            TreeReader_Expr(table, FIRSTCHILD(tree), nasm, func);
            CodeWriter_Return_Expr(nasm);
            CodeWriter_stackFrame_end(nasm, func);
            CodeWriter_Return(nasm);
        }
    } else {
        CodeWriter_stackFrame_end(nasm, func);
        CodeWriter_Return(nasm);
    }
}

static void _Instr_Assignation(const ProgramSymbolTable* table,
                               Tree tree, FILE* nasm,
                               const FunctionSymbolTable* func) {
    TreeReader_Expr(table, SECONDCHILD(tree), nasm, func);
    CodeWriter_WriteVar(nasm, FIRSTCHILD(tree), table, func);
}

static void _Instr_If(const ProgramSymbolTable* table,
                      Tree tree, FILE* nasm,
                      const FunctionSymbolTable* func) {
    assert(tree->label == If);

    int if_number = GLOBAL_CMP++;
    TreeReader_Expr(table, FIRSTCHILD(tree), nasm, func);
    CodeWriter_If_Init(nasm, if_number);
    TreeReader_SuiteInst(table, SECONDCHILD(tree), func, nasm);
    CodeWriter_If_Else(nasm, if_number);
    if (THIRDCHILD(tree)) {
        TreeReader_SuiteInst(table, THIRDCHILD(tree), func, nasm);
    }
    CodeWriter_If_End(nasm, if_number);
}

static void _Instr_While(const ProgramSymbolTable* table,
                         Tree tree, FILE* nasm,
                         const FunctionSymbolTable* func) {
    int while_number = GLOBAL_CMP++;

    CodeWriter_While_Init(nasm, while_number);

    TreeReader_Expr(table, FIRSTCHILD(tree), nasm, func);
    CodeWriter_While_Eval(nasm, while_number);

    TreeReader_SuiteInst(table, SECONDCHILD(tree), func, nasm);
    CodeWriter_While_End(nasm, while_number);
}
