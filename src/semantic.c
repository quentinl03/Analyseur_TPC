#include "semantic.h"

#include <assert.h>


static ErrorType _Semantic_Expr(
    Tree tree,
    const FunctionSymbolTable* func,
    const ProgramSymbolTable* prog
);

#define IS_FUNCTION_CALL_NODE(node) ( \
        (node)->firstChild != NULL && \
        ((node)->firstChild->label == EmptyArgs || (node)->firstChild->label == ListExp))

static ErrorType _Semantic_FunctionCall(
    Tree tree,
    const FunctionSymbolTable* caller,
    const ProgramSymbolTable* prog
) {
    assert(tree->label == Ident);
    assert(IS_FUNCTION_CALL_NODE(tree));

    const FunctionSymbolTable* calleefst = FunctionSymbolTable_get_from_name(prog, tree->att.ident);
    ErrorType err = ERR_NONE;

    if (!FunctionSymbolTable_is_defined_before_use(caller, calleefst)) {
        CodeError_print(
            (CodeError){
                .err = ERR_USE_UNDEFINED_FUNCTION,
                .column = 0,
                .line = tree->lineno,
            },
            "implicit declaration of function '%s'",
            tree->att.ident
        );
    }

    // TODO : Check argument types

    if (tree->firstChild->label != EmptyArgs) {
        // Check function arguments
        int i = 0;
        for (Node* arg = tree->firstChild->firstChild; arg;
             arg = arg->nextSibling, ++i) {
            err |= _Semantic_Expr(arg, caller, prog);
        }
    }

    return err;
}

static ErrorType _Semantic_Expr(
    Tree tree,
    const FunctionSymbolTable* func,
    const ProgramSymbolTable* prog
) {
    ErrorType err = ERR_NONE;

    switch (tree->label) {
    case Addsub:
    case Divstar:
        err |= _Semantic_Expr(FIRSTCHILD(tree), func, prog);
        err |= _Semantic_Expr(SECONDCHILD(tree), func, prog);
        break;
    case Ident:;
        const Symbol* sym = SymbolTable_resolve_from_node(prog, func, tree);
        if (IS_FUNCTION_CALL_NODE(tree)) {
            if (sym->symbol_type == SYMBOL_FUNCTION) {
                err |= _Semantic_FunctionCall(tree, func, prog);
            }
            else {
                CodeError_print(
                    (CodeError) {
                        .err = (err |= ERR_SEM_IS_NOT_CALLABLE),
                        .line = tree->lineno,
                        .column = 0,
                    },
                    "called object '%s' is not a function",
                    sym->identifier
                );
            }
        }
        // Use of function identifier without calling it is not allowed
        else if (!IS_FUNCTION_CALL_NODE(tree) && sym->symbol_type == SYMBOL_FUNCTION) {
            CodeError_print(
                (CodeError){
                    .err = (err |= ERR_FUNCTION_AS_RVALUE),
                    .line = tree->lineno,
                    .column = 0
                },
                "Pointer to function '%s' used as rvalue (not allowed)",
                sym->identifier
            );
        }
        else if (
            (sym->symbol_type == SYMBOL_POINTER_TO_ARRAY) ||
            (sym->symbol_type == SYMBOL_ARRAY)
        ) {
            // TODO : Check array access
        }
        break;
    default:
        break;
    }

    return err;
}

/**
 * @brief Check return validity
 * - A function with a return value should have a expression as a child
 * - A function without a return value should not have a child
 * 
 * @param tree Return node
 * @param func 
 * @param prog 
 * @return ErrorType 
 */
static ErrorType _Semantic_Return(
    Tree tree,
    const FunctionSymbolTable* func,
    const ProgramSymbolTable* prog
) {
    assert(tree->label == Return);
    const Symbol* fsym = SymbolTable_get(&prog->globals, func->identifier);
    ErrorType err = ERR_NONE;

    // The function returns void, but there is a return value
    if (fsym->type == type_void) {
        if (FIRSTCHILD(tree) != NULL) {
            CodeError_print(
                (CodeError) {
                    .err = (err |= ERR_RETURN_TYPE_NON_VOID),
                    .line = tree->lineno,
                    .column = 0,
                },
                "void function '%s' should not return a value",
                func->identifier
            );
        }
    }

    else {
        // The function returns non-void, but threre is no return value
        if (FIRSTCHILD(tree) == NULL) {
            CodeError_print(
                (CodeError) {
                    .err = (err |= ERR_RETURN_TYPE_VOID),
                    .line = tree->lineno,
                    .column = 0,
                },
                "non-void function '%s' should return a value",
                func->identifier
            );
        }
        else {
            err |= _Semantic_Expr(FIRSTCHILD(tree), func, prog);
        }
    }

    return err;
}

static ErrorType _Semantic_Assignation(
    Tree tree,
    const FunctionSymbolTable* func,
    const ProgramSymbolTable* prog
) {
    assert(tree->label == Assignation);
    ErrorType err = ERR_NONE;

    // TODO : Check if left side is a lvalue, and its type
    err |= _Semantic_Expr(SECONDCHILD(tree), func, prog);

    return err;
}

static ErrorType _Semantic_SuiteInstr(
    Tree tree,
    const FunctionSymbolTable* func,
    const ProgramSymbolTable* prog
) {
    assert(tree->label == SuiteInstr);
    ErrorType err = ERR_NONE;
    bool has_return = false;

    for (Node* child = tree->firstChild; child != NULL; child = child->nextSibling) {
        switch (child->label) {
        case Return:
            has_return = true;
            err |= _Semantic_Return(child, func, prog);
            break;
        case Assignation:
            err |= _Semantic_Assignation(child, func, prog);
            break;
        case Ident:
            err |= _Semantic_FunctionCall(child, func, prog);
            break;
        default:
            break;
        }
    }

    if (!has_return) {
        CodeError_print(
            (CodeError) {
                .err = ERR_RETURN_TYPE_VOID,
                .line = tree->lineno,
                .column = 0,
            },
            "missing return statement in function '%s' returning non-void",
            func->identifier
        );
        err |= ERR_RETURN_TYPE_VOID;
    }

    return err;
}

static ErrorType _Semantic_DeclFonct(Tree tree, const ProgramSymbolTable* prog) {
    assert(tree->label == DeclFonct);
    FunctionSymbolTable* func = FunctionSymbolTable_get_from_name(
        prog,
        // DeclFonct->EnTeteFonct->Ident
        FIRSTCHILD(tree)->firstChild->nextSibling->att.ident
    );
    ErrorType err = ERR_NONE;
    err |= _Semantic_SuiteInstr(SECONDCHILD(tree)->firstChild->nextSibling, func, prog);
    // TODO : Vérifier l'existence d'un return
    return err;
}

static ErrorType _Semantic_DeclFoncts(Tree tree, const ProgramSymbolTable* prog) {
    assert(tree->label == DeclFoncts);
    ErrorType err = ERR_NONE;

    // On parcourt les noeux DeclFonct
    for (Node* child = tree->firstChild;
         child != NULL;
         child = child->nextSibling
    ) {
        err |= _Semantic_DeclFonct(child, prog);
    }

    return err;
}

ErrorType static Semantic_check_main(const ProgramSymbolTable* prog) {
    ErrorType err = ERR_NONE;
    FunctionSymbolTable* fst_main;

    if ((fst_main = FunctionSymbolTable_get_from_name(prog, "main")) == NULL) {
        CodeError_print(
            (CodeError) {
                .err = (err |= ERR_MAIN_UNAVAILABLE),
                .line = 0,
                .column = 0,
            },
            "undefined reference to 'main'"
        );
    } else {
        const Symbol* sym_main = SymbolTable_get(&prog->globals, "main");
        if (fst_main->ret_type != type_num) {
            CodeError_print(
                (CodeError) {
                    .err = (err |= ERR_MAIN_RETURN_TYPE),
                    .line = sym_main->lineno,
                    .column = 0,
                },
                "'main' must return 'int'"
            );
        }
        if (FunctionSymbolTable_get_param_count(fst_main) > 0) {
            CodeError_print(
                (CodeError) {
                    .err = (err |= ERR_MAIN_PARAM),
                    .line = sym_main->lineno,
                    .column = 0,
                },
                "'main' must not have any parameter (void)"
            );
        }
    }

    return err;
}

ErrorType Semantic_check(Tree tree, const ProgramSymbolTable* prog) {
    assert(tree->label == Prog);
    ErrorType err = ERR_NONE;
    err |= _Semantic_DeclFoncts(SECONDCHILD(tree), prog);
    err |= Semantic_check_main(prog);

    return err;
}