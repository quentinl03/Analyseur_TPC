%{
#include <ctype.h>
#include <stdio.h>
#include "../src/tree.h"

void yyerror(char *msg);
int yylex();
extern unsigned int nbline;
extern unsigned int nbchar;
%}
%union {
    struct Node* node;
    char byte;
    int num;
    char ident[64];
    char comp[3];
    char key_word[10];
}
%type <node> Prog DeclVars Declarateurs DeclFoncts DeclFonct EnTeteFonct Parametres ListTypVar Corps
%type <node> SuiteInstr Instr Exp TB FB M E T F LValue Arguments ListExp Variable DeclArray ListTypVarElem DeclFonctArray ArrayLR
%token <byte> ADDSUB DIVSTAR CHARACTER
%token <num> NUM
%token <ident> TYPE IDENT VOID RETURN IF ELSE WHILE
%token <comp> OR AND EQ ORDER
%%
Prog:  DeclVars DeclFoncts {$$ = makeNode(Prog);
                           addChild($$,$1);
                           addChild($$,$2);
                           printTree($$);};
    ;
DeclVars:
       DeclVars TYPE Declarateurs ';'   {$$ = $1;
                                        Node * i = makeNode(Type);
                                        addChild(i, $3);
                                        addChild($$, i);};
    |                                   {$$ = makeNode(DeclVars);};
    ;

DeclFonctArray:
    TYPE IDENT '[' ']'                  {$$ = makeNode(DeclFonctArray);
                                        Node* type = makeNode(Type);
                                        addChild($$, type);
                                        Node* ident = makeNode(Ident);
                                        addChild(type, ident);};
    ;

DeclArray:
    IDENT '[' NUM ']'                   {$$ = makeNode(Ident);
                                        addChild($$, makeNode(Num));};

ArrayLR:
    IDENT '[' Exp ']'                   {$$ = makeNode(Ident);
                                        addChild($$, $3);};
    ;

Variable:
       DeclArray                        {$$ = $1;};
    |  IDENT                            {$$ = makeNode(Ident);};
    ;

Declarateurs:
       Declarateurs ',' Variable        {$$ = $1;
                                        addSibling($$, $3);};
    |  Variable                         {$$ = $1;};
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
                                        addChild($$,makeNode(Type));
                                        addChild($$,makeNode(Ident));
                                        addChild($$,$4);};
    |  VOID IDENT '(' Parametres ')'    {$$ = makeNode(EnTeteFonct);
                                        addChild($$,makeNode(Void));
                                        addChild($$,makeNode(Ident));
                                        addChild($$,$4);};
    ;

Parametres:
       VOID                             {$$ = makeNode(Void);};
    |  ListTypVar                       {$$ = $1;};
    ;

ListTypVarElem:
        DeclFonctArray                  {$$ = $1;}
    |   TYPE IDENT                      {$$ = makeNode(Type);
                                        addChild($$, makeNode(Ident));}

ListTypVar:
       ListTypVar ',' ListTypVarElem    {$$ = $1;
                                        addSibling($$,$3);};
    |  ListTypVarElem                   {$$ = $1;}
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
       LValue '=' Exp ';'               {$$ = makeNode(Assignation); //pas eq(==) mais = donc a changer 
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
                                        addChild($$, $1);
                                        addChild($$, $3);};
    |  M                                {$$ = $1;};
    ;
M   :  M ORDER E                        {$$ = makeNode(Order);
                                        addChild($$, $1);
                                        addChild($$, $3);};
    |  E                                {$$ = $1;};
    ;
E   :  E ADDSUB T                       {$$ = makeNode(Addsub);
                                        addChild($$, $1);
                                        addChild($$, $3);};
    |  T                                {$$ = $1;};
    ;    
T   :  T DIVSTAR F                      {$$ = makeNode(Divstar);
                                        addChild($$, $1);
                                        addChild($$, $3);};
    |  F                                {$$ = $1;};
    ;
F   :  ADDSUB F                         {$$ = makeNode(Addsub);
                                        addChild($$,$2);};
    |  '!' F                            {$$ = makeNode(Not);
                                        addChild($$,$2);};
    |  '(' Exp ')'                      {$$ = $2;};
    |  NUM                              {$$ = makeNode(Num);};
    |  CHARACTER                        {$$ = makeNode(Character);};
    |  LValue                           {$$ = $1;};
    |  IDENT '(' Arguments  ')'         {$$ = makeNode(Ident);
                                        addChild($$,$3);};
    ;
LValue:
       IDENT                            {$$ = makeNode(Ident);};
    |  ArrayLR                          {$$ = $1;}
    ;
Arguments:
       ListExp                          {$$ = $1;};
    |                                   {$$ = makeNode(EmptyInstr);};
    ;
ListExp:
       ListExp ',' Exp                  {$$ = $1;
                                        addChild($$,$3);};
    |  Exp                              {$$ = makeNode(ListExp);
                                        addChild($$,$1);};
    ;
%%
void yyerror(char* msg){
    fprintf(stderr, "%s: line %u column %u\n", msg, nbline, nbchar);
}

int main(int argc, char** argv){
    return yyparse();
    /* lire argv = yyin = fopen(file); */

}
