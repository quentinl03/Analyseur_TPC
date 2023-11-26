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
}
%type <node> Prog DeclVars Declarateurs DeclFoncts DeclFonct EnTeteFonct Parametres ListTypVar Corps
%type <node> SuiteInstr Instr Exp TB FB M E T F LValue Arguments ListExp
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
                                        Node * i = makeNode(TYPE);
                                        addChild(i, $3);
                                        addChild($$, i);};
    |                                   {$$ = makeNode(DeclVars);};
    ;
Declarateurs:
       Declarateurs ',' IDENT           {$$ = $1;
                                        addSibling($$, makeNode(IDENT));};
    |  IDENT                            {$$ = makeNode(IDENT);};
    ;
DeclFoncts:
       DeclFoncts DeclFonct             {$$ = $1;
                                        addChild($$,$1);};
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
                                        addChild($$,makeNode(TYPE));
                                        addChild($$,makeNode(IDENT));
                                        addChild($$,$4);};
    |  VOID IDENT '(' Parametres ')'    {$$ = makeNode(EnTeteFonct);
                                        addChild($$,makeNode(VOID));
                                        addChild($$,makeNode(IDENT));
                                        addChild($$,$4);};
    ;
Parametres:
       VOID                             {$$ = makeNode(VOID);};
    |  ListTypVar                       {$$ = $1;};
    ;
ListTypVar:
       ListTypVar ',' TYPE IDENT        {$$ = $1;
                                        Node* i = makeNode(TYPE);
                                        addSibling(i,makeNode(IDENT));
                                        addChild($$,i);};
    |  TYPE IDENT                       {$$ = makeNode(ListTypVar);
                                        Node* i = makeNode(TYPE);
                                        addSibling(i,makeNode(IDENT));
                                        addChild($$,i);};
    ;
Corps: '{' DeclVars SuiteInstr '}'      {$$ = makeNode(Corps);
                                        addChild($$,$2);
                                        addChild($$,$3);};
    ;
SuiteInstr:
       SuiteInstr Instr                 {$$ = $1;
                                        if($2)
                                            addChild($$,$2);};
    |                                   {$$ = makeNode(SuiteInstr);};
    ;
Instr:
       LValue '=' Exp ';'               {$$ = makeNode('=');
                                        addChild($$, $1);
                                        addChild($$, $3);};
    |  IF '(' Exp ')' Instr             {$$ = makeNode(IF);
                                        addChild($$,$3);
                                        if($5)
                                            addChild($$,$5);};
    |  IF '(' Exp ')' Instr ELSE Instr  {$$ = makeNode(IF);
                                        addChild($$,$3);
                                        if($5)
                                            addChild($$,$5);
                                        Node* i = makeNode(ELSE);
                                        if($7)
                                            addChild($$,$7);
                                        addSibling($$,i);};
    |  WHILE '(' Exp ')' Instr          {$$ = makeNode(WHILE);
                                        addChild($$,$3);
                                        if($5)
                                            addChild($$,$5);};
    |  IDENT '(' Arguments ')' ';'      {$$ = makeNode(IDENT);
                                        if($3)
                                            addChild($$,$3);};
    |  RETURN Exp ';'                   {$$ = makeNode(RETURN);
                                        addChild($$,$2);};
    |  RETURN ';'                       {$$ = makeNode(RETURN);};
    |  '{' SuiteInstr '}'               {$$ = $2;};
    |  ';'                              {;};
    ;
Exp :  Exp OR TB                        {$$ = makeNode(OR);
                                        addChild($$, $1);
                                        addChild($$, $3);};
    |  TB                               {$$ = $1;};
    ;
TB  :  TB AND FB                        {$$ = makeNode(AND);
                                        addChild($$, $1);
                                        addChild($$, $3);};
    |  FB                               {$$ = $1;};
    ;
FB  :  FB EQ M                          {$$ = makeNode(EQ);
                                        addChild($$, $1);
                                        addChild($$, $3);};
    |  M                                {$$ = $1;};
    ;
M   :  M ORDER E                        {$$ = makeNode(ORDER);
                                        addChild($$, $1);
                                        addChild($$, $3);};
    |  E                                {$$ = $1;};
    ;
E   :  E ADDSUB T                       {$$ = makeNode(ADDSUB);
                                        addChild($$, $1);
                                        addChild($$, $3);};
    |  T                                {$$ = $1;};
    ;    
T   :  T DIVSTAR F                      {$$ = makeNode(DIVSTAR);
                                        addChild($$, $1);
                                        addChild($$, $3);};
    |  F                                {$$ = $1;};
    ;
F   :  ADDSUB F                         {$$ = makeNode(ADDSUB);
                                        addChild($$,$2);};
    |  '!' F                            {$$ = makeNode('!');
                                        addChild($$,$2);};
    |  '(' Exp ')'                      {$$ = $2;};
    |  NUM                              {$$ = makeNode(NUM);};
    |  CHARACTER                        {$$ = makeNode(CHARACTER);};
    |  LValue                           {$$ = $1;};
    |  IDENT '(' Arguments  ')'         {$$ = makeNode(IDENT);
                                        addChild($$,$3);};
    ;
LValue:
       IDENT                            {$$ = makeNode(IDENT);};
    ;
Arguments:
       ListExp                          {$$ = $1;};
    |                                   {;};
    ;
ListExp:
       ListExp ',' Exp                  {$$ = $1;
                                        addChild($$,$3);};
    |  Exp                              {$$ = makeNode(ListExp);
                                        addChild($$,$1);};
    ;
%%
void yyerror(char* msg){
    fprintf(stderr, "%s: line %u column %u", msg, nbline, nbchar);
}

int main(int argc, char** argv){
    return yyparse();
}
