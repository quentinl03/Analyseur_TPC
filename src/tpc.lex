%{
#include "tpc.tab.h"
int nbline = 1;
%}

%option nounput
%option noinput
%x COMMENT

/* letter [a-zA-Z]|\xc3[\x80-\xbf]|\xC5[\x92-\x93] */
IDENTIFIER                  [a-zA-Z_][a-zA-Z0-9_]*
SEPARATOR                   [\t\r\n ]
ESCAPED_ASCII               \\['0rnt]
LITERAL                     \'({ESCAPED_ASCII}|[ -&]|[(-~])\'

%%

"/*"                        {BEGIN COMMENT;};
<COMMENT>(.|{SEPARATOR})    {/* Do nothing */};
<COMMENT>"*/"               {BEGIN INITIAL;};

"//"(.|\t)*                 {/* Do nothing */};

int|char                    {return TYPE;};
void                        {return VOID;};
if                          {return IF;};
else                        {return ELSE;};
while                       {return WHILE;};
return                      {return RETURN;};
[*/%]                       {return DIVSTAR;};
[+-]                        {return ADDSUB;};
"<"|">"|"<="|">="           {return ORDER;};
"||"                        {return OR;};
"&&"                        {return AND;};
"=="|"!="                   {return EQ;};

[1-9][0-9]*|0               {return NUM;};
({IDENTIFIER})              {return IDENT;};
({LITERAL})                 {return CHARACTER;}; 

\n                          {++nbline;};
({SEPARATOR})               {/* Do nothing */};
.                           {return yytext[0];};
<<EOF>>                     {return 0;}; 

%%
