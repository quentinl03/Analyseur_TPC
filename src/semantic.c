#include "semantic.h"

#include <assert.h>
#include <stdio.h>  // TODO : Remove after debug

#include "tree.h"

static ExprReturn _Semantic_Expr(Tree tree,
                                 const FunctionSymbolTable* func,
                                 const ProgramSymbolTable* prog);

#define IS_FUNCTION_CALL_NODE(node) ( \
    (node)->firstChild != NULL &&     \
    ((node)->firstChild->label == EmptyArgs || (node)->firstChild->label == ListExp))

static ErrorType _Semantic_FunctionCall(Tree tree,
                                        const FunctionSymbolTable* caller,
                                        const ProgramSymbolTable* prog) {
    assert(tree->label == Ident);
    assert(IS_FUNCTION_CALL_NODE(tree));

    const FunctionSymbolTable* calleefst = FunctionSymbolTable_get_from_name(prog, tree->att.ident);
    ErrorType err = ERR_NONE;

    if (!FunctionSymbolTable_is_defined_before_use(caller, calleefst)) {
        CodeError_print(
            (CodeError){
                .err = (err |= ERR_USE_UNDEFINED_FUNCTION),
                .column = 0,
                .line = tree->lineno,
            },
            "implicit declaration of function '%s'",
            tree->att.ident);
    }

    // TODO : Check argument types

    int i = 0;
    Node* arg;
    // Check function arguments
    for (arg = tree->firstChild->firstChild; arg; arg = arg->nextSibling, ++i) {
        if (i >= FunctionSymbolTable_get_param_count(calleefst)) {
            CodeError_print(
                (CodeError){
                    .err = (err |= ERR_INVALID_PARAM_COUNT),
                    .line = arg->lineno,
                    .column = 0,
                },
                "too many arguments to function call '%s', expected %d",
                tree->att.ident,
                FunctionSymbolTable_get_param_count(calleefst));
            break;
        }

        ExprReturn ret = _Semantic_Expr(arg, caller, prog);
        // Get the i-th parameter of the function being called for type checking
        const Symbol* arg_sym = FunctionSymbolTable_get_param(calleefst, i);

        if (ret.type == type_num && arg_sym->type == type_byte) {
            CodeError_print(
                (CodeError){
                    .err = (err |= WARN_IMPLICIT_INT_TO_CHAR),
                    .line = arg->lineno,
                    .column = 0,
                },
                "passing 'int' to parameter of type 'char'");
        }
        err |= ret.err;
    }

    if (i < FunctionSymbolTable_get_param_count(calleefst)) {
        CodeError_print(
            (CodeError){
                .err = (err |= ERR_INVALID_PARAM_COUNT),
                .line = tree->lineno,
                .column = 0,
            },
            "too few arguments to function call '%s', expected %d, have %d",
            tree->att.ident,
            FunctionSymbolTable_get_param_count(calleefst),
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
                                     const FunctionSymbolTable* func,
                                     const ProgramSymbolTable* prog) {
    assert(tree->label == ArrayLR);
    Symbol* sym = SymbolTable_resolve_from_node(prog, func, tree);
    if (sym->symbol_type == SYMBOL_ARRAY || sym->symbol_type == SYMBOL_POINTER_TO_ARRAY) {
        // We check if the indexing expression is valid
        return (ExprReturn){_Semantic_Expr(FIRSTCHILD(tree), func, prog).err, sym->type};
    }
    else {
        CodeError_print(
            (CodeError){
                .err = ERR_SUBSCRIPT_NOT_ARRAY,
                .line = tree->lineno,
                .column = 0,
            },
            "subscripted value '%s' is not an array or pointer to array",
            sym->identifier
        );
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
 * @brief Calculate the type of an expression
 * Operations between a char and an int will result in an int type (implicit cast)
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
                                 const FunctionSymbolTable* func,
                                 const ProgramSymbolTable* prog) {
    ErrorType error = ERR_NONE;
    ExprReturn left, right;

    switch (tree->label) {
        case AddsubU:
            left = _Semantic_Expr(FIRSTCHILD(tree), func, prog);
            return (ExprReturn){.err = left.err, .type = _cast_types(left.type, type_num)};
        case Addsub:
        case Divstar:;
            left = _Semantic_Expr(FIRSTCHILD(tree), func, prog);
            right = _Semantic_Expr(SECONDCHILD(tree), func, prog);
            return (ExprReturn){.err = left.err | right.err, .type = _cast_types(left.type, right.type)};
        case Ident:;
            const Symbol* sym = SymbolTable_resolve_from_node(prog, func, tree);
            if (IS_FUNCTION_CALL_NODE(tree)) {              // true function call
                if (sym->symbol_type == SYMBOL_FUNCTION) {  // verify if it's a function
                    return (ExprReturn){
                        .err = _Semantic_FunctionCall(tree, func, prog),
                        .type = sym->type};
                }
                CodeError_print(
                    (CodeError){
                        .err = (error |= ERR_SEM_IS_NOT_CALLABLE),
                        .line = tree->lineno,
                        .column = 0,
                    },
                    "called object '%s' is not a function",
                    sym->identifier);
                return (ExprReturn){.err = error, .type = sym->type};
            }
            else if (sym->symbol_type == SYMBOL_VALUE) {
                return (ExprReturn){.err = error, .type = sym->type};
            }
            else {
                CodeError_print(
                    (CodeError){
                        .err = (error |= ERR_NOT_AN_RVALUE),
                        .line = tree->lineno,
                        .column = 0,
                    },
                    "'%s' is not an rvalue",
                    sym->identifier);
                return (ExprReturn){.err = error, .type = sym->type};
            }
            return (ExprReturn){.err = error, .type = sym->type};
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
            return (ExprReturn){.err = left.err | right.err, .type = _cast_types(left.type, right.type)};
        case Not:
            left = _Semantic_Expr(FIRSTCHILD(tree), func, prog);
            return (ExprReturn){.err = left.err, .type = _cast_types(left.type, type_num)};
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
                                  const FunctionSymbolTable* func,
                                  const ProgramSymbolTable* prog) {
    assert(tree->label == Return);
    const Symbol* fsym = SymbolTable_get(&prog->globals, func->identifier);
    ErrorType err = ERR_NONE;

    // The function returns non-void, but threre is no return value
    if (FIRSTCHILD(tree) == NULL && fsym->type != type_void) {
        CodeError_print(
            (CodeError){
                .err = (err |= ERR_RETURN_TYPE_VOID),
                .line = tree->lineno,
                .column = 0,
            },
            "non-void function '%s' should return a value",
            func->identifier);
    }
    if (FIRSTCHILD(tree)) {
        ExprReturn ret = _Semantic_Expr(FIRSTCHILD(tree), func, prog);
        err |= ret.err;
        if (fsym->type == type_byte && ret.type == type_num) {
            CodeError_print(
                (CodeError){
                    .err = (err |= WARN_IMPLICIT_INT_TO_CHAR),
                    .line = tree->lineno,
                    .column = 0,
                },
                "return type mismatch in function '%s' (cast from 'int' to 'char')",
                func->identifier
            );
        }
        else if (fsym->type == type_void && ret.type == type_void) {
            CodeError_print(
                (CodeError){
                    .err = (err |= ERR_RETURN_TYPE_VOID),
                    .line = tree->lineno,
                    .column = 0,
                },
                "In function '%s', ISO C forbids 'return' with expression, "
                "even if the expression evaluates to void",
                func->identifier
            );
        }
        else if (fsym->type == type_void) {
            CodeError_print(
                (CodeError){
                    .err = (err |= ERR_RETURN_TYPE_NON_VOID),
                    .line = tree->lineno,
                    .column = 0,
                },
                "void function '%s' should not return a value",
                func->identifier
            );
        }
    }

    return err;
}

static ErrorType _Semantic_Assignation(Tree tree,
                                       const FunctionSymbolTable* func,
                                       const ProgramSymbolTable* prog) {
    assert(tree->label == Assignation);
    ErrorType err = ERR_NONE;
    const Symbol* lvalue = SymbolTable_resolve_from_node(prog, func, FIRSTCHILD(tree));

    if (FIRSTCHILD(tree)->label == ArrayLR) {
        err |= _Semantic_ArrayLR(FIRSTCHILD(tree), func, prog).err;
    }
    else if (lvalue->symbol_type != SYMBOL_VALUE) {
        CodeError_print(
            (CodeError){
                .err = (err |= ERR_NOT_AN_LVALUE),
                .line = tree->lineno,
                .column = 0,
            },
            "'%s' is not an lvalue",
            lvalue->identifier
        );
        return err;
    }

    ExprReturn ret = _Semantic_Expr(SECONDCHILD(tree), func, prog);
    err |= ret.err;

    if (lvalue->type == type_byte && ret.type == type_num) {
        CodeError_print(
            (CodeError){
                .err = (err |= WARN_IMPLICIT_INT_TO_CHAR),
                .line = tree->lineno,
                .column = 0,
            },
            "assignation type mismatch in function '%s' to variable '%s' (cast from 'int' to 'char')",
            func->identifier,
            lvalue->identifier
        );
    }
    else if (ret.type == type_void) {
        CodeError_print(
            (CodeError){
                .err = (err |= ERR_NO_VOID_EXPRESSION),
                .line = tree->lineno,
                .column = 0,
            },
            "assigning to variable '%s' from incompatible type 'void' in function '%s'",
            lvalue->identifier,
            func->identifier
        );
    }

    return err;
}

static ErrorType _Semantic_SuiteInstr(Tree tree,
                                      const FunctionSymbolTable* func,
                                      const ProgramSymbolTable* prog) {
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
            case If:
            case While:
                err |= _Semantic_Expr(FIRSTCHILD(child), func, prog).err;
                break;
            default:
                break;
        }
    }

    // Check if the function has a return statement (naive approch, should be improved)
    // Doesn't work with blocks of code (if, while)
    if (!has_return) {
        CodeError_print(
            (CodeError){
                .err = ERR_RETURN_TYPE_VOID,
                .line = tree->lineno,
                .column = 0,
            },
            "missing return statement in function '%s' returning non-void",
            func->identifier);
        err |= ERR_RETURN_TYPE_VOID;
    }

    return err;
}

static ErrorType _Semantic_DeclFonct(Tree tree,
                                     const ProgramSymbolTable* prog) {
    assert(tree->label == DeclFonct);
    FunctionSymbolTable* func = FunctionSymbolTable_get_from_name(
        prog,
        // DeclFonct->EnTeteFonct->Ident
        FIRSTCHILD(tree)->firstChild->nextSibling->att.ident);
    ErrorType err = ERR_NONE;
    err |= _Semantic_SuiteInstr(SECONDCHILD(tree)->firstChild->nextSibling,
                                func, prog);
    return err;
}

static ErrorType _Semantic_DeclFoncts(Tree tree,
                                      const ProgramSymbolTable* prog) {
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

ErrorType static Semantic_check_main(const ProgramSymbolTable* prog) {
    ErrorType err = ERR_NONE;
    FunctionSymbolTable* fst_main;

    if ((fst_main = FunctionSymbolTable_get_from_name(prog, "main")) == NULL) {
        CodeError_print(
            (CodeError){
                .err = (err |= ERR_MAIN_UNAVAILABLE),
                .line = 0,
                .column = 0,
            },
            "undefined reference to 'main'");
    } else {
        const Symbol* sym_main = SymbolTable_get(&prog->globals, "main");
        if (fst_main->ret_type != type_num) {
            CodeError_print(
                (CodeError){
                    .err = (err |= ERR_MAIN_RETURN_TYPE),
                    .line = sym_main->lineno,
                    .column = 0,
                },
                "'main' must return 'int'");
        }
        if (FunctionSymbolTable_get_param_count(fst_main) > 0) {
            CodeError_print(
                (CodeError){
                    .err = (err |= ERR_MAIN_PARAM),
                    .line = sym_main->lineno,
                    .column = 0,
                },
                "'main' must not have any parameter (void)");
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