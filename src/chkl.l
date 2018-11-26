%{

  #include <stdlib.h>
  #include <unistd.h>
  #include <string.h>
  //#include <stdbool.h>
  #include "src/chkl_parser.h"
  #include "chkl_yacc.h"
  #include "src/utils.h"

  static bool isVoid = false;

%}
%option noyywrap
%option nounput
%option noinput
%option nodefault
%option warn

%x NUM DREF VOID VOID1

EOL                     (\r|\n|\n\r|\r\n)
STRING                                [^\n\r:|]+
VOIDSTRING                        [^\n\r|]+
VOIDSTRING1                        [^\n\r]+
DATAREF                                [^\n\r:|\[\]\(\)&]+
COLON                   :
LBRACKET                \[
RBRACKET                \]
LPAREN                  \(
RPAREN                  \)
POW                     \*\*
PLUS                    \+
MINUS                   \-
MUL                     \*
DIV                     \/
ABS_DIF                 ><
NEQ                     !
EQ                      =
LT                      <
GT                      >
PIPE                    \|
OR                      \|\|
POS_DIF                 \+>
NEG_DIF                 -<
DOT                     \.
AND                     &&
NUMBER                  [0-9]+

%%
sw_checklist                 {isVoid = false; return(TOKEN_CHECKLIST);}
sw_item                      {isVoid = false; return(TOKEN_ITEM);}
sw_iteminfo                  {isVoid = false; return(TOKEN_ITEMINFO);}
sw_itemvoid                  {isVoid = true;  return(TOKEN_ITEMVOID);}
sw_show                      {isVoid = false; BEGIN(DREF); return(TOKEN_SHOW);}
sw_rcolsize                  {isVoid = false; return(TOKEN_RCOLSIZE);}
sw_remark                    {isVoid = false; return(TOKEN_REMARK);}
sw_continue[[:blank:]]*      {isVoid = false; return(TOKEN_CONTINUE);}

<*>{COLON}                   {if(isVoid){BEGIN(VOID);} return(TOKEN_COLON);}

<NUM>{NEQ}                   {return(TOKEN_NE);}
<NUM>{LT}{EQ}                {return(TOKEN_LE);}
<NUM>{GT}{EQ}                {return(TOKEN_GE);}
<NUM>{EQ}                    {return(TOKEN_EQ);}
<NUM>{LT}                    {return(TOKEN_LT);}
<NUM>{GT}                    {return(TOKEN_GT);}
<NUM>{POS_DIF}               {return(TOKEN_POS_DIF);}
<NUM>{NEG_DIF}               {return(TOKEN_NEG_DIF);}
<NUM>{ABS_DIF}               {return(TOKEN_ABS_DIF);}

<NUM>{PLUS}                  {return(TOKEN_PLUS);}
<NUM>{MINUS}                 {return(TOKEN_MINUS);}
<NUM>{MUL}                   {return(TOKEN_MUL);}
<NUM>{DIV}                   {return(TOKEN_DIV);}
<NUM>{POW}                   {return(TOKEN_POW);}
<NUM>\{[^}\r\n]+\}           {
                              //skip opening brace, don't include the closing one
                              size_t len = strlen(yytext);
                              chkllval.str = strdup(yytext + 1);
                              chkllval.str[len - 2] = '\0';
                              return(TOKEN_DREF);
                             }
<NUM>{NUMBER}                {
                               chkllval.str=strdup(yytext);
                               return(TOKEN_NUMBER);
                             }
<NUM>{DOT}{NUMBER}           {
                               chkllval.str=strdup(yytext);
                               return(TOKEN_FRAC);
                             }
<NUM>[eE][+-]?{NUMBER}       {
                               chkllval.str=strdup(yytext);
                               return(TOKEN_EXPONENT);
                             }
<NUM>[a-zA-Z][a-zA-Z0-9_]+   {
                               chkllval.str = strdup(yytext);
                               return(TOKEN_STRING);
                             }
<NUM>\(double\)              {return(TOKEN_TO_DOUBLE);}
<NUM>\(float\)               {return(TOKEN_TO_FLOAT);}
<NUM>\(int\)                 {return(TOKEN_TO_INT);}
<NUM>,                       {return(TOKEN_COMA);}

<INITIAL,NUM,DREF>{PIPE}     {return(TOKEN_PIPE);}
<DREF>{OR}                   {return(TOKEN_OR);}
<DREF>{AND}                  {return(TOKEN_AND);}
<DREF>{LBRACKET}             {return(TOKEN_LEFT_BRACKET);}
<DREF>{RBRACKET}             {return(TOKEN_RIGHT_BRACKET);}
<DREF,NUM>{LPAREN}           {return(TOKEN_LEFT_PARENTHESIS);}
<DREF,NUM>{RPAREN}           {return(TOKEN_RIGHT_PARENTHESIS);}
<*>^[[:blank:]]+#[^\r\n]*    {}
<*>#[^\r\n]*                 {}
<*>{EOL}                     {
                               BEGIN(INITIAL);
                               isVoid = false;
                               ++chkllineno;
                               //fprintf(stderr, ">>>>>>>>>>Initial!\n");
                             }
<*>[[:blank:]]+              /*eat whitespaces*/
{STRING}                     {
                               chkllval.str=strdup(yytext);
                               //fprintf(stderr, "\nSTRING:'%s'\n", yytext);
                               return(TOKEN_STRING);
                             }

<DREF>{DATAREF}              {
                               chkllval.str=strdup(yytext);
                               //fprintf(stderr, "\nDSTRING:'%s'\n", yytext);
                               return(TOKEN_STRING);
                             }
<VOID>{VOIDSTRING}           {
                               chkllval.str=strdup(yytext);
                               //fprintf(stderr, "\nSTRING:'%s'\n", yytext);
                               return(TOKEN_STRING);
                             }
<VOID>{PIPE}                 {
                               BEGIN(VOID1);
                               return(TOKEN_PIPE);
                             }
<VOID1>{VOIDSTRING1}         {//ugly cludge
                               chkllval.str=strdup(yytext);
                               //fprintf(stderr, "\nSTRING:'%s'\n", yytext);
                               return(TOKEN_STRING);
                             }
<*>.                         {
                               xcDebug("XChecklist: Unmatched character '%s' at line %d!\n",
                                 chkltext, chkllineno);
                             }
%%

void expect_number()
{
  BEGIN(NUM);
  //fprintf(stderr, ">>>>>>>>>Expect number\n");
}

void expect_dataref()
{
  BEGIN(DREF);
  //fprintf(stderr, ">>>>>>>>>Expect dataref\n");
}

void expect_nothing()
{
  BEGIN(INITIAL);
  //fprintf(stderr, ">>>>>>>>>Expect nothing\n");
}

