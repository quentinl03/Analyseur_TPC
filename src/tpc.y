%{
#include <ctype.h>
#include <stdio.h>
#include "../src/tree.h"
#include "../src/parser.h"
#include "../src/error.h"

void yyerror(Node** abr, char *msg);
int yylex();
extern unsigned int nbline;
extern unsigned int nbchar;
%}
%parse-param {Node ** abr}
%union {
    struct Node* node;
    char byte;
    int num;
    char ident[64];
    char key_word[5];
}
%type <node> Prog DeclVars Declarateurs DeclFoncts DeclFonct 
%type <node> EnTeteFonct Parametres ListTypVar Corps
%type <node> SuiteInstr Instr Exp TB FB M E T F LValue 
%type <node> Arguments ListExp DeclArray DeclFonctArray ArrayLR
%token <byte> ADDSUB DIVSTAR CHARACTER
%token <num> NUM
%token <ident> IDENT VOID RETURN IF ELSE WHILE
%token <key_word> OR AND EQ ORDER TYPE

%destructor { free($$); } DeclVars Declarateurs DeclFoncts DeclFonct 
%destructor { free($$); } EnTeteFonct Parametres ListTypVar Corps
%destructor { free($$); } SuiteInstr Instr Exp TB FB M E T F LValue 
%destructor { free($$); } Arguments ListExp DeclArray DeclFonctArray ArrayLR

%expect 1
/* Character in key_word not char -> in case of \n, \t, \r and \0*/
%%
Prog:  DeclVars DeclFoncts              {*abr = makeNode(Prog);
                                        addChild(*abr,$1);
                                        addChild(*abr,$2);
                                        };
    ;
DeclVars:
       DeclVars TYPE Declarateurs ';'   {$$ = $1;
                                        Node * i = makeNode(Type);
                                        addAttributKeyWord(i, $2);
                                        addChild(i, $3);
                                        addChild($$, i);};
    |                                   {$$ = makeNode(DeclVars);};
    ;
Declarateurs:
       Declarateurs ',' IDENT           {$$ = $1;
                                        Node* i = makeNode(Ident);
                                        addAttributIdent(i, $3);
                                        addSibling($$, i);};
    |  Declarateurs ',' DeclArray       {$$ = $1;
                                        addSibling($$, $3);};
    |  IDENT                            {$$ = makeNode(Ident);
                                        addAttributIdent($$, $1);};
    |  DeclArray                        {$$ = $1;};
    ;
DeclFonctArray:
    TYPE IDENT '[' ']'                  {$$ = makeNode(DeclFonctArray);
                                        addAttributKeyWord($$, $1);
                                        Node* ident = makeNode(Ident);
                                        addAttributIdent(ident, $2);
                                        addChild($$, ident);};
    ;

DeclArray:
    IDENT '[' NUM ']'                   {$$ = makeNode(DeclArray);
                                        addAttributIdent($$,$1);
                                        Node* num = makeNode(Num);
                                        addAttributNum(num, $3);
                                        addChild($$, num);};

ArrayLR:
    IDENT '[' Exp ']'                   {$$ = makeNode(ArrayLR);
                                        addAttributIdent($$,$1);
                                        addChild($$, $3);};
    ;
DeclFoncts:
       DeclFoncts DeclFonct             {$$ = $1;
                                        addChild($$,$2);};
    |  DeclFonct                        {$$ = makeNode(DeclFoncts);
                                        addChild($$,$1);};
    ;
DeclFonct:
       EnTeteFonct Corps                {$$ = makeNode(DeclFonct);
                                        addChild($$,$1);
                                        addChild($$,$2);};
    ;
EnTeteFonct:
       TYPE IDENT '(' Parametres ')'    {$$ = makeNode(EnTeteFonct);
                                        Node* i = makeNode(Type);
                                        addAttributKeyWord(i, $1);
                                        addChild($$, i);
                                        Node* j = makeNode(Ident);
                                        addAttributIdent(j, $2);
                                        addChild($$, j);
                                        addChild($$, $4);};
    |  VOID IDENT '(' Parametres ')'    {$$ = makeNode(EnTeteFonct);
                                        addChild($$, makeNode(Void));
                                        Node* j = makeNode(Ident);
                                        addAttributIdent(j, $2);
                                        addChild($$, j);
                                        addChild($$, $4);};
    ;

Parametres:
       VOID                             {$$ = makeNode(Void);};
    |  ListTypVar                       {$$ = $1;};
    ;
ListTypVar:
       ListTypVar ',' TYPE IDENT        {$$ = $1;
                                        Node* i = makeNode(Type);
                                        addAttributKeyWord(i, $3);
                                        Node* j = makeNode(Ident);
                                        addAttributIdent(j, $4);
                                        addChild(i, j);
                                        addChild($$,i);};
    |  ListTypVar ',' DeclFonctArray    {$$ = $1;
                                        addChild($$, $3);};
    |  TYPE IDENT                       {$$ = makeNode(ListTypVar);
                                        Node* i = makeNode(Type);
                                        addAttributKeyWord(i, $1);
                                        Node* j = makeNode(Ident);
                                        addAttributIdent(j, $2);
                                        addChild(i, j);
                                        addChild($$, i);};
    |  DeclFonctArray                   {$$ = makeNode(ListTypVar);
                                        addChild($$, $1);};
    ;
Corps: '{' DeclVars SuiteInstr '}'      {$$ = makeNode(Corps);
                                        addChild($$,$2);
                                        addChild($$,$3);};
    ;
SuiteInstr:
       SuiteInstr Instr                 {$$ = $1;
                                        addChild($$,$2);};
    |                                   {$$ = makeNode(SuiteInstr);};
    ;
Instr:
       LValue '=' Exp ';'               {$$ = makeNode(Assignation);
                                        addChild($$, $1);
                                        addChild($$, $3);};
    |  IF '(' Exp ')' Instr             {$$ = makeNode(If);
                                        addChild($$,$3);
                                        addChild($$,$5);};
    |  IF '(' Exp ')' Instr ELSE Instr  {$$ = makeNode(If);
                                        addChild($$,$3);
                                        addChild($$,$5);
                                        addChild($$,$7);};
    |  WHILE '(' Exp ')' Instr          {$$ = makeNode(While);
                                        addChild($$,$3);
                                        addChild($$,$5);};
    |  IDENT '(' Arguments ')' ';'      {$$ = makeNode(Ident);
                                        addAttributIdent($$, $1);
                                        addChild($$,$3);};
    |  RETURN Exp ';'                   {$$ = makeNode(Return);
                                        addChild($$,$2);};
    |  RETURN ';'                       {$$ = makeNode(Return);};
    |  '{' SuiteInstr '}'               {$$ = $2;};
    |  ';'                              {$$ = makeNode(EmptyInstr);};
    ;
Exp :  Exp OR TB                        {$$ = makeNode(Or);
                                        addChild($$, $1);
                                        addChild($$, $3);};
    |  TB                               {$$ = $1;};
    ;
TB  :  TB AND FB                        {$$ = makeNode(And);
                                        addChild($$, $1);
                                        addChild($$, $3);};
    |  FB                               {$$ = $1;};
    ;
FB  :  FB EQ M                          {$$ = makeNode(Eq);
                                        addAttributKeyWord($$, $2);
                                        addChild($$, $1);
                                        addChild($$, $3);};
    |  M                                {$$ = $1;};
    ;
M   :  M ORDER E                        {$$ = makeNode(Order);
                                        addAttributKeyWord($$, $2);
                                        addChild($$, $1);
                                        addChild($$, $3);};
    |  E                                {$$ = $1;};
    ;
E   :  E ADDSUB T                       {$$ = makeNode(Addsub);
                                        addAttributByte($$, $2);
                                        addChild($$, $1);
                                        addChild($$, $3);};
    |  T                                {$$ = $1;};
    ;    
T   :  T DIVSTAR F                      {$$ = makeNode(Divstar);
                                        addAttributByte($$, $2);
                                        addChild($$, $1);
                                        addChild($$, $3);};
    |  F                                {$$ = $1;};
    ;
F   :  ADDSUB F                         {$$ = makeNode(AddsubU);
                                        addAttributByte($$, $1);
                                        addChild($$,$2);};
    |  '!' F                            {$$ = makeNode(Not);
                                        addChild($$,$2);};
    |  '(' Exp ')'                      {$$ = $2;};
    |  NUM                              {$$ = makeNode(Num);
                                        addAttributNum($$, $1);};
    |  CHARACTER                        {$$ = makeNode(Character);
                                        addAttributByte($$, $1);};
    |  LValue                           {$$ = $1;};
    |  IDENT '(' Arguments  ')'         {$$ = makeNode(Ident);
                                        addAttributIdent($$, $1);
                                        addChild($$,$3);};
    ;
LValue:
       IDENT                            {$$ = makeNode(Ident);
                                        addAttributIdent($$, $1);};
    |  ArrayLR                          {$$ = $1;}
    ;
Arguments:
       ListExp                          {$$ = $1;};
    |                                   {$$ = makeNode(EmptyArgs);};
    ;
ListExp:
       ListExp ',' Exp                  {$$ = $1;
                                        addChild($$,$3);};
    |  Exp                              {$$ = makeNode(ListExp);
                                        addChild($$,$1);};
    ;
%%
void yyerror(Node** abr, char* msg) {
    fprintf(stderr, "%s: line %u column %u\n", msg, nbline, nbchar);
}

ErrorType parser_bison(FILE* f, Node** abr) {
    extern FILE* yyin;
    yyin = f;
    int retcode = yyparse(abr);
    return (
        retcode == 1 ? ERR_PARSE_SYNTAX
        : retcode == 2 ? ERR_NO_MEMORY
        : ERR_NONE
    );
}
