/* tree.h */

#define FOREACH_NODE(NODE) \
    NODE(Prog)             \
    NODE(DeclVars)         \
    NODE(DeclFoncts)       \
    NODE(DeclFonct)        \
    NODE(Declarateurs)     \
    NODE(EnTeteFonct)      \
    NODE(Corps)            \
    NODE(Parametres)       \
    NODE(ListTypVar)       \
    NODE(SuiteInstr)       \
    NODE(Instr)            \
    NODE(EmptyInstr)       \
    NODE(LValue)           \
    NODE(Exp)              \
    NODE(Arguments)        \
    NODE(TB)               \
    NODE(FB)               \
    NODE(M)                \
    NODE(E)                \
    NODE(T)                \
    NODE(F)                \
    NODE(ListExp)

#define GENERATE_ENUM(ENUM) ENUM,
#define GENERATE_STRING(STRING) #STRING,

typedef enum {
    FOREACH_NODE(GENERATE_ENUM)
} label_t;

// typedef enum {
//     E,
//     T,
//     divstar,
//     id
//     /* list all other node labels, if any */
//     /* The list must coincide with the string array in tree.c */
//     /* To avoid listing them twice, see https://stackoverflow.com/a/10966395 */
// } label_t;

typedef struct Node {
    label_t label;
    struct Node *firstChild, *nextSibling;
    int lineno;
} Node;

Node *makeNode(label_t label);
void addSibling(Node *node, Node *sibling);
void addChild(Node *parent, Node *child);
void deleteTree(Node *node);
void printTree(Node *node);

#define FIRSTCHILD(node) node->firstChild
#define SECONDCHILD(node) node->firstChild->nextSibling
#define THIRDCHILD(node) node->firstChild->nextSibling->nextSibling
