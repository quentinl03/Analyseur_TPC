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
    NODE(ListExp)          \
    NODE(Addsub)           \
    NODE(Divstar)          \
    NODE(Character)        \
    NODE(Num)              \
    NODE(Type)             \
    NODE(Ident)            \
    NODE(Void)             \
    NODE(Return)           \
    NODE(If)               \
    NODE(Else)             \
    NODE(While)            \
    NODE(Or)               \
    NODE(And)              \
    NODE(Eq)               \
    NODE(Order)            \
    NODE(Not)              \
    NODE(Assignation)

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

typedef union {
    char byte;
    int num;
    char ident[64];
    char key_word[5];
} Attribut;

typedef enum {
    type_char,
    type_int,
    type_ident,
} type_t;

typedef struct Node {
    label_t label;
    struct Node *firstChild, *nextSibling;
    Attribut att;
    int lineno;
} Node;

Node *makeNode(label_t label);
void addAttribut(Node *node, Attribut att);
void addSibling(Node *node, Node *sibling);
void addChild(Node *parent, Node *child);
void deleteTree(Node *node);
void printTree(Node *node);

#define FIRSTCHILD(node) node->firstChild
#define SECONDCHILD(node) node->firstChild->nextSibling
#define THIRDCHILD(node) node->firstChild->nextSibling->nextSibling