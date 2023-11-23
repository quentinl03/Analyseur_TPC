%{
#include "tpc.tab.h"
unsigned int nbline = 1;
unsigned int nbchar = 1;

#define CHAR_INC (nbchar += yyleng)
#define CHAR_RST (nbchar = 0)

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

"/*"                        {CHAR_INC; BEGIN COMMENT;};
<COMMENT>\n                 {nbline++; CHAR_RST;};
<COMMENT>(.|{SEPARATOR})    {CHAR_INC;};
<COMMENT>"*/"               {CHAR_INC; BEGIN INITIAL;};

"//"(.|\t)*                 {CHAR_INC;};

int|char                    {CHAR_INC; return TYPE;};
void                        {CHAR_INC; return VOID;};
if                          {CHAR_INC; return IF;};
else                        {CHAR_INC; return ELSE;};
while                       {CHAR_INC; return WHILE;};
return                      {CHAR_INC; return RETURN;};
[*/%]                       {CHAR_INC; return DIVSTAR;};
[+-]                        {CHAR_INC; return ADDSUB;};
"<"|">"|"<="|">="           {CHAR_INC; return ORDER;};
"||"                        {CHAR_INC; return OR;};
"&&"                        {CHAR_INC; return AND;};
"=="|"!="                   {CHAR_INC; return EQ;};

[1-9][0-9]*|0               {CHAR_INC; return NUM;};
({IDENTIFIER})              {CHAR_INC; return IDENT;};
({LITERAL})                 {CHAR_INC; return CHARACTER;}; 

\n                          {nbline++; CHAR_RST;};
({SEPARATOR})               {CHAR_INC; };
.                           {CHAR_INC; return yytext[0];};
<<EOF>>                     {return 0;}; 

%%
