%{
#include "../src/tree.h"
#include "tpc.tab.h"
unsigned int nbline = 1;
unsigned int nbchar = 1;

#define CHAR_INC (nbchar += yyleng)
#define CHAR_RST (nbchar = 0)

static inline char litteral_to_char(char litteral[]) {
    static const char charmap[128] = {
        ['0'] = '\0',
        ['r'] = '\r',
        ['n'] = '\n',
        ['t'] = '\t',
        ['\\'] = '\\',
        ['\''] = '\'',
    };

    if (litteral[1] == '\\') {
        return charmap[(int) litteral[2]];
    } else {
        return litteral[1];
    }
}

%}

%option nounput
%option noinput
%x COMMENT

/* letter [a-zA-Z]|\xc3[\x80-\xbf]|\xC5[\x92-\x93] */
IDENTIFIER                  [a-zA-Z_][a-zA-Z0-9_]{0,63}
SEPARATOR                   [\t\r\n ]
ESCAPED_ASCII               \\['0rnt\\]

/* Allow escaped characters, and select all printable characters between
 33 and 126 included, while excluding non-escaped 
 single quote character and backslash */
LITERAL                     \'({ESCAPED_ASCII}|[^'\\[:cntrl:]])\'

%%

"/*"                        {CHAR_INC; BEGIN COMMENT;};
<COMMENT>\n                 {nbline++; CHAR_RST;};
<COMMENT>(.|{SEPARATOR})    {CHAR_INC;};
<COMMENT>"*/"               {CHAR_INC; BEGIN INITIAL;};

"//"(.|\t)*                 {CHAR_INC;};

int|char                    {strcpy(yylval.key_word, yytext);
                            CHAR_INC; return TYPE;};
void                        {CHAR_INC; return VOID;};
if                          {CHAR_INC; return IF;};
else                        {CHAR_INC; return ELSE;};
while                       {CHAR_INC; return WHILE;};
return                      {CHAR_INC; return RETURN;};
[*/%]                       {yylval.byte = yytext[0];
                            CHAR_INC; return DIVSTAR;};
[+-]                        {yylval.byte = yytext[0];
                            CHAR_INC; return ADDSUB;};
"<"|">"|"<="|">="           {strcpy(yylval.key_word, yytext);
                            CHAR_INC; return ORDER;};
"||"                        {CHAR_INC; return OR;};
"&&"                        {CHAR_INC; return AND;};
"=="|"!="                   {strcpy(yylval.key_word, yytext);
                            CHAR_INC; return EQ;};

[1-9][0-9]*|0               {yylval.num = atoi(yytext);
                            CHAR_INC; return NUM;};
({IDENTIFIER})              {strcpy(yylval.ident, yytext);
                            CHAR_INC; return IDENT;};
({LITERAL})                 {yylval.byte = litteral_to_char(yytext);
                            CHAR_INC; return CHARACTER;}; 

\n                          {nbline++; CHAR_RST;};
({SEPARATOR})               {CHAR_INC; };
.                           {yylval.byte = yytext[0];
                            CHAR_INC; return yytext[0];};
<<EOF>>                     {return 0;}; 

%%
