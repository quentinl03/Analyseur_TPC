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
    char key_word[5];
}
%type <node> Prog DeclVars Declarateurs DeclFoncts DeclFonct EnTeteFonct Parametres ListTypVar Corps
%type <node> SuiteInstr Instr Exp TB FB M E T F LValue Arguments ListExp
%token <byte> ADDSUB DIVSTAR CHARACTER
%token <num> NUM
%token <ident> IDENT VOID RETURN IF ELSE WHILE
%token <key_word> OR AND EQ ORDER TYPE
%%
Prog:  DeclVars DeclFoncts              {$$ = makeNode(Prog);
                                        addChild($$,$1);
                                        addChild($$,$2);
                                        printTree($$);};
    ;
DeclVars:
       DeclVars TYPE Declarateurs ';'   {$$ = $1;
                                        Node * i = makeNode(Type);
                                        addAttribut(i, $2, type_key_word);
                                        addChild(i, $3);
                                        addChild($$, i);};
    |                                   {$$ = makeNode(DeclVars);};
    ;
Declarateurs:
       Declarateurs ',' IDENT           {$$ = $1;
                                        Node* i = makeNode(Ident);
                                        addAttribut(i, $3, type_ident);
                                        addSibling($$, i);};
    |  IDENT                            {$$ = makeNode(Ident);
                                        addAttribut($$, $1, type_ident);};
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
                                        addAttribut(i, $1, type_key_word);
                                        addChild($$, i);
                                        Node* j = makeNode(Ident);
                                        addAttribut(j, $2, type_ident);
                                        addChild($$, j);
                                        addChild($$, $4);};
    |  VOID IDENT '(' Parametres ')'    {$$ = makeNode(EnTeteFonct);
                                        addChild($$, makeNode(Void));
                                        Node* j = makeNode(Ident);
                                        addAttribut(j, $1, type_ident);
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
                                        addAttribut(i, $3, type_key_word);
                                        Node* j = makeNode(Ident);
                                        addAttribut(j, $4, type_ident);
                                        addChild(i, j);
                                        addChild($$,i);};
    |  TYPE IDENT                       {$$ = makeNode(ListTypVar);
                                        Node* i = makeNode(Type);
                                        addAttribut(i, $1, type_key_word);
                                        Node* j = makeNode(Ident);
                                        addAttribut(j, $2, type_ident);
                                        addChild(i, j);
                                        addChild($$, i);};
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
                                        Node* i = makeNode(Else);
                                        addChild(i,$7);
                                        addSibling($$,i);};
    |  WHILE '(' Exp ')' Instr          {$$ = makeNode(While);
                                        addChild($$,$3);
                                        addChild($$,$5);};
    |  IDENT '(' Arguments ')' ';'      {$$ = makeNode(Ident);
                                        addAttribut($$, $1, type_ident);
                                        addChild($$,$3);};
    |  RETURN Exp ';'                   {$$ = makeNode(Return);
                                        addChild($$,$2);};
    |  RETURN ';'                       {$$ = makeNode(Return);};
    |  '{' SuiteInstr '}'               {$$ = $2;};
    |  ';'                              {$$ = makeNode(EmptyInstr);};
    ;
Exp :  Exp OR TB                        {$$ = makeNode(Or);
                                        addAttribut($$, $2, type_key_word);
                                        addChild($$, $1);
                                        addChild($$, $3);};
    |  TB                               {$$ = $1;};
    ;
TB  :  TB AND FB                        {$$ = makeNode(And);
                                        addAttribut($$, $2, type_key_word);
                                        addChild($$, $1);
                                        addChild($$, $3);};
    |  FB                               {$$ = $1;};
    ;
FB  :  FB EQ M                          {$$ = makeNode(Eq);
                                        addAttribut($$, $2, type_key_word);
                                        addChild($$, $1);
                                        addChild($$, $3);};
    |  M                                {$$ = $1;};
    ;
M   :  M ORDER E                        {$$ = makeNode(Order);
                                        addAttribut($$, $2, type_key_word);
                                        addChild($$, $1);
                                        addChild($$, $3);};
    |  E                                {$$ = $1;};
    ;
E   :  E ADDSUB T                       {$$ = makeNode(Addsub);
                                        addAttribut($$, $2, type_key_word);
                                        addChild($$, $1);
                                        addChild($$, $3);};
    |  T                                {$$ = $1;};
    ;    
T   :  T DIVSTAR F                      {$$ = makeNode(Divstar);
                                        addAttribut($$, $2, type_key_word);
                                        addChild($$, $1);
                                        addChild($$, $3);};
    |  F                                {$$ = $1;};
    ;
F   :  ADDSUB F                         {$$ = makeNode(Addsub);
                                        addAttribut($$, $2, type_key_word);
                                        addChild($$,$2);};
    |  '!' F                            {$$ = makeNode(Not);
                                        addChild($$,$2);};
    |  '(' Exp ')'                      {$$ = $2;};
    |  NUM                              {$$ = makeNode(Num);
                                        addAttribut($$, $1, type_num);};
    |  CHARACTER                        {$$ = makeNode(Character);
                                        addAttribut($$, $1, type_byte);};
    |  LValue                           {$$ = $1;};
    |  IDENT '(' Arguments  ')'         {$$ = makeNode(Ident);
                                        addAttribut($$, $1, type_ident);
                                        addChild($$,$3);};
    ;
LValue:
       IDENT                            {$$ = makeNode(Ident);
                                        addAttribut($$, $1, type_ident);};
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
