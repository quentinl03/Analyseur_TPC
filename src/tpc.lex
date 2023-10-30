%{
#include "tpc.tab.h"
int nbline = 1;
%}
%option nounput
%option noinput
/* letter [a-zA-Z]|\xc3[\x80-\xbf]|\xC5[\x92-\x93] */
identificator [a-zA-Z_][a-zA-Z0-9_]*
separator [\t\r ]
%%
int|char            {return TYPE;};
void                {return VOID;};
[*/%]               {return DIVSTAR;};
[+-]                {return ADDSUB;};
"<"|">"|"<="|">="   {return ORDER;};
"||"                {return OR;};
"&&"                {return AND;};
==|!=               {return EQ;};
if                  {return IF;};
else                {return ELSE;};
while               {return WHILE;};
return              {return RETURN;};
({identificator}) {return IDENT;};

({separator}) {/*Do nothing*/};
\n {++nbline;};

. {return yytext[0];};
<<EOF>> {return 0;}; 

%%