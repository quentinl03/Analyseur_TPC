#include "semantic.h"

#include <assert.h>
#include <stdio.h>  // TODO : Remove after debug

#include "tree.h"

static ExprReturn _Semantic_Expr(Tree tree,
                                 const FunctionST* func,
                                 const ProgramST* prog);

static ErrorType _Semantic_SuiteInstr(Tree tree,
                                      const FunctionST* func,
                                      const ProgramST* prog,
                                      bool is_root);

#define IS_FUNCTION_CALL_NODE(node) (          \
    (node)->firstChild != NULL &&              \
    ((node)->firstChild->label == EmptyArgs || \
     (node)->firstChild->label == ListExp))

#define IS_ONLY_IDENTIFIER(node) ( \
    (node)->label == Ident &&      \
    (node)->firstChild == NULL)

static ErrorType _Semantic_FunctionCall(Tree tree,
                                        const FunctionST* caller,
                                        const ProgramST* prog) {
    assert(tree->label == Ident);
    assert(IS_FUNCTION_CALL_NODE(tree));

    ErrorType err = ERR_NONE;

    const Symbol* sym = ST_resolve_from_node(prog, caller, tree);
    if (sym->symbol_type != SYMBOL_FUNCTION) {
        CodeError_print(
            (CodeError){
                .err = ADD_ERR(err, ERR_SEM_IS_NOT_CALLABLE),
                .line = tree->lineno,
                .column = tree->column,
            },
            "called object '%s' is not a function",
            sym->identifier);
        return err;
    }

    const FunctionST* calleefst = FunctionST_get_from_name(prog,
                                                           tree->att.ident);
    if (!FunctionST_is_defined_before_use(caller, calleefst)) {
        CodeError_print(
            (CodeError){
                .err = ADD_ERR(err, WARN_USE_UNDEFINED_FUNCTION),
                .column = tree->column,
                .line = tree->lineno,
            },
            "implicit declaration of function '%s'",
            tree->att.ident);
    }

    int i = 0;
    Node* arg;
    // Check function arguments
    for (arg = tree->firstChild->firstChild; arg; arg = arg->nextSibling, ++i) {
        if (i >= FunctionST_get_param_count(calleefst)) {
            // Check if we have more arguments than expected
            CodeError_print(
                (CodeError){
                    .err = ADD_ERR(err, ERR_INVALID_PARAM_COUNT),
                    .line = arg->lineno,
                    .column = tree->column,
                },
                "too many arguments to function call '%s', expected %d",
                tree->att.ident,
                FunctionST_get_param_count(calleefst));
            break;
        }
        // Get the i-th parameter of the function being called for type checking
        const Symbol* param_sym = FunctionST_get_param(calleefst, i);

        // If we have a node with only an identifier, it could be an array
        if (IS_ONLY_IDENTIFIER(arg) || param_sym->symbol_type == SYMBOL_ARRAY) {
            const Symbol* arg_sym = ST_resolve_from_node(prog, caller, arg);

            if (arg_sym->symbol_type == SYMBOL_ARRAY ||
                param_sym->symbol_type == SYMBOL_ARRAY) {
                // If one of them is an array,
                // we need to check that the other is also an array
                if (param_sym->symbol_type != arg_sym->symbol_type) {
                    CodeError_print(
                        (CodeError){
                            .err = ADD_ERR(err, ERR_MISMATCH_ARRAY_TYPE),
                            .line = arg->lineno,
                            .column = tree->column,
                        },
                        "expected %s, got %s",
                        SymbolType_to_str(param_sym->symbol_type),
                        SymbolType_to_str(arg_sym->symbol_type));
                }
                // We check if the array types are the same
                // (int[] to char[] is forbidden)
                else if (arg_sym->type != param_sym->type) {
                    CodeError_print(
                        (CodeError){
                            .err = ADD_ERR(err, ERR_INVALID_ARRAY_TYPE),
                            .line = arg->lineno,
                            .column = tree->column,
                        },
                        "expected array of type '%s', got '%s'",
                        Symbol_get_type_str(param_sym->type),
                        Symbol_get_type_str(arg_sym->type));
                }
                continue;
            }
        }
        // The argument is not an array, and so an expression
        ExprReturn ret = _Semantic_Expr(arg, caller, prog);

        /* We check that the expression type can be implicitly casted
           to the parameter type (char to int is allowed, not the opposite) */
        if (param_sym->symbol_type == SYMBOL_VALUE &&
            ret.type == type_num &&
            param_sym->type == type_byte) {
            CodeError_print(
                (CodeError){
                    .err = ADD_ERR(err, WARN_IMPLICIT_INT_TO_CHAR),
                    .line = arg->lineno,
                    .column = arg->column,
                },
                "'int' to parameter of type 'char'");
        }
        err |= ret.err;
    }

    // Check if we have less arguments than expected
    if (i < FunctionST_get_param_count(calleefst)) {
        CodeError_print(
            (CodeError){
                .err = ADD_ERR(err, ERR_INVALID_PARAM_COUNT),
                .line = tree->lineno,
                .column = tree->column,
            },
            "too few arguments to function call '%s', expected %d, have %d",
            tree->att.ident,
            FunctionST_get_param_count(calleefst),
            i);
    }

    return err;
}

/**
 * @brief Check if the array is a valid RValue (array or pointer to array,
 * and the indexing expression is valid)
 *
 * @param tree ArrayLR node
 * @param func
 * @param prog
 * @return ExprReturn
 */
static ExprReturn _Semantic_ArrayLR(Tree tree,
                                    const FunctionST* func,
                                    const ProgramST* prog) {
    assert(tree->label == ArrayLR);
    Symbol* sym = ST_resolve_from_node(prog, func, tree);
    if (sym->symbol_type == SYMBOL_ARRAY) {
        // We check if the indexing expression is valid
        ExprReturn ret = _Semantic_Expr(FIRSTCHILD(tree), func, prog);
        return (ExprReturn){.err = ret.err, .type = sym->type};
    } else {
        CodeError_print(
            (CodeError){
                .err = ERR_SUBSCRIPT_NOT_ARRAY,
                .line = tree->lineno,
                .column = tree->column,
            },
            "subscripted value '%s' is not an array or pointer to array",
            sym->identifier);
        return (ExprReturn){ERR_SUBSCRIPT_NOT_ARRAY, sym->type};
    }
}

/**
 * @brief Cast types to a common type
 * Any operation between a void and another type will result in a void type
 * Any operation between a char or int will result in an int type
 *
 * @param t1
 * @param t2
 * @return type_t
 */
static type_t _cast_types(type_t t1, type_t t2) {
    if (t1 == type_void || t2 == type_void) {
        return type_void;
    }
    return type_num;
}

/**
 * @brief Check if the identifier is a valid RValue
 * Don't use on ArrayLR nodes
 *
 * @param tree
 * @param func
 * @param prog
 * @return ExprReturn
 */
static ExprReturn _Semantic_IdentRValue(Tree tree,
                                        const FunctionST* func,
                                        const ProgramST* prog) {
    assert(tree->label == Ident);

    ErrorType error = ERR_NONE;
    const Symbol* sym = ST_resolve_from_node(prog, func, tree);

    // The user is trying to call a function
    if (IS_FUNCTION_CALL_NODE(tree)) {
        if (sym->type != type_void) {
            return (ExprReturn){
                .err = _Semantic_FunctionCall(tree, func, prog),
                .type = sym->type};
        }
        // The function is void, we can't use it as an rvalue
    } else if (sym->symbol_type == SYMBOL_VALUE) {
        return (ExprReturn){.err = ERR_NONE, .type = sym->type};
    }
    CodeError_print(
        (CodeError){
            .err = (error |= ERR_NOT_AN_RVALUE),
            .line = tree->lineno,
            .column = tree->column - 1,
        },
        "'%s' is not an rvalue",
        sym->identifier);
    return (ExprReturn){.err = error, .type = sym->type};
}

/**
 * @brief Calculate the type of an expression
 * Operations between a char and an int will result in an int type
 * (implicit cast)
 * If any of the operands is a void, the result will be a void type. We avoid
 * edges cases where the void type is used in an operation like :
 * void f(void) {return;}
 * int main(void) {int a; a = 2 + f();}
 *
 * @param tree
 * @param func
 * @param prog
 * @return ExprReturn
 */
static ExprReturn _Semantic_Expr(Tree tree,
                                 const FunctionST* func,
                                 const ProgramST* prog) {
    ErrorType error = ERR_NONE;
    ExprReturn left, right;

    switch (tree->label) {
        case AddsubU:
            left = _Semantic_Expr(FIRSTCHILD(tree), func, prog);
            return (ExprReturn){.err = left.err,
                                .type = _cast_types(left.type, type_num)};
        case Addsub:
        case Divstar:;
            left = _Semantic_Expr(FIRSTCHILD(tree), func, prog);
            right = _Semantic_Expr(SECONDCHILD(tree), func, prog);
            return (ExprReturn){.err = left.err | right.err,
                                .type = _cast_types(left.type, right.type)};
        case Ident:;
            return _Semantic_IdentRValue(tree, func, prog);
        case EmptyArgs:
            return (ExprReturn){.err = error, .type = type_void};
        case Num:
            return (ExprReturn){.err = error, .type = type_num};
        case Character:
            return (ExprReturn){.err = error, .type = type_byte};
        case Eq:
        case Or:
        case And:
        case Order:
            left = _Semantic_Expr(FIRSTCHILD(tree), func, prog);
            right = _Semantic_Expr(SECONDCHILD(tree), func, prog);
            return (ExprReturn){.err = left.err | right.err,
                                .type = _cast_types(left.type, right.type)};
        case Not:
            left = _Semantic_Expr(FIRSTCHILD(tree), func, prog);
            return (ExprReturn){.err = left.err,
                                .type = _cast_types(left.type, type_num)};
        case ArrayLR:
            return _Semantic_ArrayLR(tree, func, prog);
        default:
            assert(0 && "We shouldn't be there (_Sematinic_Expr)");
    }
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
static ErrorType _Semantic_Return(Tree tree,
                                  const FunctionST* func,
                                  const ProgramST* prog) {
    assert(tree->label == Return);
    const Symbol* fsym = ST_get(&prog->globals, func->identifier);
    ErrorType err = ERR_NONE;

    // The function returns non-void, but threre is no return value
    if (FIRSTCHILD(tree) == NULL && fsym->type != type_void) {
        CodeError_print(
            (CodeError){
                .err = ADD_ERR(err, ERR_MUST_RETURN_VALUE),
                .line = tree->lineno,
                .column = tree->column,
            },
            "non-void function '%s' must return a value",
            func->identifier);
    }
    if (FIRSTCHILD(tree)) {
        ExprReturn ret = _Semantic_Expr(FIRSTCHILD(tree), func, prog);
        err |= ret.err;
        if (fsym->type == type_byte && ret.type == type_num) {
            CodeError_print(
                (CodeError){
                    .err = ADD_ERR(err, WARN_IMPLICIT_INT_TO_CHAR),
                    .line = tree->lineno,
                    .column = tree->column,
                },
                "return type mismatch in function '%s' "
                "(cast from 'int' to 'char')",
                func->identifier);
        } else if (fsym->type == type_void && ret.type == type_void) {
            CodeError_print(
                (CodeError){
                    .err = ADD_ERR(err, ERR_RETURN_VOID_EXPR),
                    .line = tree->lineno,
                    .column = tree->column,
                },
                "In function '%s', ISO C forbids 'return' with expression, "
                "even if the expression evaluates to void",
                func->identifier);
        } else if (fsym->type == type_void) {
            CodeError_print(
                (CodeError){
                    .err = ADD_ERR(err, ERR_RETURN_TYPE_NON_VOID),
                    .line = tree->lineno,
                    .column = tree->column,
                },
                "void function '%s' should not return a value",
                func->identifier);
        }
    }

    return err;
}

static ErrorType _Semantic_Assignation(Tree tree,
                                       const FunctionST* func,
                                       const ProgramST* prog) {
    assert(tree->label == Assignation);
    ErrorType err = ERR_NONE;
    const Symbol* lvalue = ST_resolve_from_node(prog, func, FIRSTCHILD(tree));

    if (FIRSTCHILD(tree)->label == ArrayLR) {
        err |= _Semantic_ArrayLR(FIRSTCHILD(tree), func, prog).err;
    } else if (lvalue->symbol_type != SYMBOL_VALUE) {
        CodeError_print(
            (CodeError){
                .err = ADD_ERR(err, ERR_NOT_AN_LVALUE),
                .line = FIRSTCHILD(tree)->lineno,
                .column = FIRSTCHILD(tree)->column - 1,
            },
            "'%s' is not an lvalue",
            lvalue->identifier);
        return err;
    }

    ExprReturn ret = _Semantic_Expr(SECONDCHILD(tree), func, prog);
    err |= ret.err;

    if (lvalue->type == type_byte && ret.type == type_num) {
        CodeError_print(
            (CodeError){
                .err = ADD_ERR(err, WARN_IMPLICIT_INT_TO_CHAR),
                .line = tree->lineno,
                .column = tree->column,
            },
            "assignation type mismatch in function '%s' to variable '%s' "
            "(cast from 'int' to 'char')",
            func->identifier,
            lvalue->identifier);
    } else if (ret.type == type_void) {
        CodeError_print(
            (CodeError){
                .err = ADD_ERR(err, ERR_NO_VOID_EXPRESSION),
                .line = tree->lineno,
                .column = tree->column,
            },
            "assigning to variable '%s' "
            "from incompatible type 'void' in function '%s'",
            lvalue->identifier,
            func->identifier);
    }

    return err;
}

static ErrorType _Semantic_If(Tree tree,
                              const FunctionST* func,
                              const ProgramST* prog) {
    assert(tree->label == If);
    ErrorType err = ERR_NONE;

    err |= _Semantic_Expr(FIRSTCHILD(tree), func, prog).err;
    err |= _Semantic_SuiteInstr(SECONDCHILD(tree), func, prog, false);

    if (THIRDCHILD(tree)) {
        err |= _Semantic_SuiteInstr(THIRDCHILD(tree), func, prog, false);
    }

    return err;
}

static ErrorType _Semantic_While(Tree tree,
                                 const FunctionST* func,
                                 const ProgramST* prog) {
    assert(tree->label == While);
    ErrorType err = ERR_NONE;

    err |= _Semantic_Expr(FIRSTCHILD(tree), func, prog).err;
    err |= _Semantic_SuiteInstr(SECONDCHILD(tree), func, prog, false);

    return err;
}

static ErrorType _Semantic_SuiteInstr(Tree tree,
                                      const FunctionST* func,
                                      const ProgramST* prog,
                                      bool is_root) {
    assert(
        tree->label == SuiteInstr || tree->label == Return ||
        tree->label == Assignation || tree->label == Ident ||
        tree->label == If || tree->label == While ||
        tree->label == EmptyInstr);

    ErrorType err = ERR_NONE;
    bool has_return = false;

    Node* child = tree->label == SuiteInstr ? FIRSTCHILD(tree) : tree;

    for (; child != NULL; child = child->nextSibling) {
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
            case SuiteInstr:
                err |= _Semantic_SuiteInstr(child, func, prog, 0);
                break;
            case EmptyInstr:
                break;
            case If:
                err |= _Semantic_If(child, func, prog);
                break;
            case While:
                err |= _Semantic_While(child, func, prog);
                break;
            default:
                assert(0 && "We shouldn't be there");
        }
    }

    // Check if the function has a return statement
    // (naive approch, should be improved)
    // Doesn't work with blocks of code (if, while)
    if (!has_return && func->ret_type != type_void && is_root) {
        CodeError_print(
            (CodeError){
                .err = ADD_ERR(err, WARN_MISSING_RETURN),
                .line = tree->lineno + 1,
                .column = 0,
            },
            "missing return statement in function '%s' returning non-void",
            func->identifier);
    }

    return err;
}

static ErrorType _Semantic_DeclFonct(Tree tree,
                                     const ProgramST* prog) {
    assert(tree->label == DeclFonct);
    FunctionST* func = FunctionST_get_from_name(
        prog,
        // DeclFonct->EnTeteFonct->Ident
        FIRSTCHILD(tree)->firstChild->nextSibling->att.ident);
    ErrorType err = ERR_NONE;
    err |= _Semantic_SuiteInstr(SECONDCHILD(tree)->firstChild->nextSibling,
                                func, prog, true);
    return err;
}

static ErrorType _Semantic_DeclFoncts(Tree tree,
                                      const ProgramST* prog) {
    assert(tree->label == DeclFoncts);
    ErrorType err = ERR_NONE;

    // On parcourt les noeux DeclFonct
    for (Node* child = tree->firstChild;
         child != NULL;
         child = child->nextSibling) {
        err |= _Semantic_DeclFonct(child, prog);
    }

    return err;
}

ErrorType static Semantic_check_main(const ProgramST* prog) {
    ErrorType err = ERR_NONE;
    FunctionST* fst_main;

    if ((fst_main = FunctionST_get_from_name(prog, "main")) == NULL) {
        CodeError_print(
            (CodeError){
                .err = ADD_ERR(err, ERR_MAIN_UNAVAILABLE),
                .line = 0,
                .column = 0,
            },
            "undefined reference to 'main'");
    } else {
        const Symbol* sym_main = ST_get(&prog->globals, "main");
        if (fst_main->ret_type != type_num) {
            CodeError_print(
                (CodeError){
                    .err = ADD_ERR(err, ERR_MAIN_RETURN_TYPE),
                    .line = sym_main->lineno,
                    .column = sym_main->column,
                },
                "'main' must return 'int'");
        }
        if (FunctionST_get_param_count(fst_main) > 0) {
            CodeError_print(
                (CodeError){
                    .err = ADD_ERR(err, ERR_MAIN_PARAM),
                    .line = sym_main->lineno,
                    .column = sym_main->column,
                },
                "'main' must not have any parameter (void)");
        }
    }

    return err;
}

ErrorType Semantic_check(Tree tree, const ProgramST* prog) {
    assert(tree->label == Prog);
    ErrorType err = ERR_NONE;
    err |= _Semantic_DeclFoncts(SECONDCHILD(tree), prog);
    err |= Semantic_check_main(prog);

    return err;
}