%{

  #include <stdlib.h>
  #include <string.h>
  //#include <stdbool.h>
  #include "chkl_yacc.h"
  #include "utils.h"
  #include "chkl_parser.h"
  
  static bool isVoid = false;

%}
%option noyywrap
%option nounput
%option noinput
%option nodefault
%option warn

%x NUM DREF VOID

EOL                     (\r|\n|\n\r|\r\n)
STRING			        [^\n\r:|]+
VOIDSTRING		        [^\n\r]+
DATAREF			        [^\n\r:|\[\]\(\)&]+
COLON                   :
LBRACKET                \[
RBRACKET                \]
LPAREN                  \(
RPAREN                  \)
NEQ                     !
EQ                      =
LT                      <
GT                      >
PIPE                    \|
OR                      \|\|
DOT                     \.
AND                     &&
NUMBER                  [+-]?[0-9]+

%%
sw_checklist            {isVoid = false; return(TOKEN_CHECKLIST);}
sw_item                 {isVoid = false; return(TOKEN_ITEM);}
sw_iteminfo             {isVoid = false; return(TOKEN_ITEMINFO);}
sw_itemvoid             {isVoid = true;  return(TOKEN_ITEMVOID);}
sw_show                 {isVoid = false; return(TOKEN_SHOW);}
sw_rcolsize             {isVoid = false; return(TOKEN_RCOLSIZE);}
sw_remark               {isVoid = false; return(TOKEN_REMARK);}
sw_continue[[:blank:]]* {isVoid = false; return(TOKEN_CONTINUE);}

<*>{COLON}	{if(isVoid){BEGIN(VOID);} return(TOKEN_COLON);}

<NUM>{NEQ}                   {return(TOKEN_NE);}
<NUM>{LT}{EQ}                {return(TOKEN_LE);}
<NUM>{GT}{EQ}                {return(TOKEN_GE);}
<NUM>{EQ}                    {return(TOKEN_EQ);}
<NUM>{LT}                    {return(TOKEN_LT);}
<NUM>{GT}                    {return(TOKEN_GT);}
<INITIAL,NUM,DREF>{PIPE}     {return(TOKEN_PIPE);}
<DREF>{OR}                   {return(TOKEN_OR);}
<DREF>{AND}                  {return(TOKEN_AND);}
<NUM>{NUMBER}                {
			  chkllval.str=strdup(yytext);
                          return(TOKEN_NUMBER);
                        }
<NUM>{DOT}{NUMBER}           {
			  chkllval.str=strdup(yytext);
                          return(TOKEN_FRAC);
                        }
<NUM>[eE]{NUMBER}            {
			  chkllval.str=strdup(yytext);
                          return(TOKEN_EXPONENT);
                        }
<DREF>{LBRACKET}                {return(TOKEN_LEFT_BRACKET);}  
<DREF>{RBRACKET}                {return(TOKEN_RIGHT_BRACKET);}  
<DREF>{LPAREN}                  {return(TOKEN_LEFT_PARENTHESIS);}  
<DREF,NUM>{RPAREN}                  {return(TOKEN_RIGHT_PARENTHESIS);}  
<*>^[[:blank:]]+#[^\r\n]*	{}
<*>#[^\r\n]*			{}
<*>{EOL}	{
                          BEGIN(INITIAL);
                          isVoid = false;
                          ++chkllineno;
                          //fprintf(stderr, ">>>>>>>>>>Initial!\n");
                        }
<*>[[:blank:]]+      /*eat whitespaces*/
{STRING}	{
			  chkllval.str=strdup(yytext);
			  //fprintf(stderr, "\nSTRING:'%s'\n", yytext);
			  return(TOKEN_STRING);
			}

<DREF>{DATAREF}	{
			  chkllval.str=strdup(yytext);
			  //fprintf(stderr, "\nDSTRING:'%s'\n", yytext);
			  return(TOKEN_STRING);
			}
<VOID>{VOIDSTRING}      {
			  chkllval.str=strdup(yytext);
			  //fprintf(stderr, "\nSTRING:'%s'\n", yytext);
			  return(TOKEN_STRING);
                        }
<*>. {
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

