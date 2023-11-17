%{
#include <ctype.h>
#include <stdio.h>

void yyerror(char *msg);
int yylex();
extern int nbline;
%}
%union {
    Node *node;
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
Prog:  DeclVars DeclFoncts {$$ = makeNode(Prog); addChild($$,$1); addChild($$,$2);
                            printTree($$);};
    ;
DeclVars:
       DeclVars TYPE Declarateurs ';'       {Node* i = makeNode(TYPE);
                                            addChild(i,$3);
                                            $$ = addSibling($1,i);};
    |                                       {$$ = makeNode(DeclVars);};
    ;
Declarateurs:
       Declarateurs ',' IDENT               {$$ = $1;
                                            addSibling($$,IDENT);};
    |  IDENT                                {$$ = makeNode(IDENT);};
    ;
DeclFoncts:
       DeclFoncts DeclFonct
    |  DeclFonct
    ;
DeclFonct:
       EnTeteFonct Corps
    ;
EnTeteFonct:
       TYPE IDENT '(' Parametres ')'
    |  VOID IDENT '(' Parametres ')'
    ;
Parametres:
       VOID
    |  ListTypVar
    ;
ListTypVar:
       ListTypVar ',' TYPE IDENT
    |  TYPE IDENT
    ;
Corps: '{' DeclVars SuiteInstr '}'
    ;
SuiteInstr:
       SuiteInstr Instr
    |
    ;
Instr:
       LValue '=' Exp ';'
    |  IF '(' Exp ')' Instr
    |  IF '(' Exp ')' Instr ELSE Instr
    |  WHILE '(' Exp ')' Instr
    |  IDENT '(' Arguments ')' ';'
    |  RETURN Exp ';'
    |  RETURN ';'
    |  '{' SuiteInstr '}'
    |  ';'
    ;
Exp :  Exp OR TB
    |  TB
    ;
TB  :  TB AND FB
    |  FB
    ;
FB  :  FB EQ M
    |  M
    ;
M   :  M ORDER E
    |  E
    ;
E   :  E ADDSUB T
    |  T
    ;    
T   :  T DIVSTAR F 
    |  F
    ;
F   :  ADDSUB F
    |  '!' F
    |  '(' Exp ')'
    |  NUM
    |  CHARACTER
    |  LValue
    |  IDENT '(' Arguments  ')'
    ;
LValue:
       IDENT
    ;
Arguments:
       ListExp
    |
    ;
ListExp:
       ListExp ',' Exp
    |  Exp
    ;
%%
void yyerror(char* msg){
    fprintf(stderr,"%s near line %d\n", msg, nbline);
}

int main(void){
    return yyparse();
}