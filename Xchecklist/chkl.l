%{

  #include <stdlib.h>
  #include <string.h>
  //#include <stdbool.h>
  #include "chkl_yacc.h"
  #include "utils.h"
  
  static int colons = 0;
%}
%option noyywrap
%option nounput
%option noinput
%option nodefault
%option warn
%option yylineno

%x ERR NUM DREF

EOL                     (\r|\n|\r\r)
STRING			        [^\n\r:|]+
DATAREF			        [^\n\r:|\[\]]+
COLON                   :
RBRACKET                \[
LBRACKET                \]
NEQ                     !
EQ                      =
LE                      <
GE                      >
PIPE                    \|
DOT                     \.
NUMBER                  [+-]?[0-9]+

%%
sw_checklist            {colons = 0; return(TOKEN_CHECKLIST);}
sw_item                 {colons = 0; return(TOKEN_ITEM);}
sw_iteminfo             {colons = 0; return(TOKEN_ITEMINFO);}
sw_itemvoid             {colons = 0; return(TOKEN_ITEMVOID);}
sw_show                 {colons = 0; return(TOKEN_SHOW);}
sw_rcolsize             {colons = 0; return(TOKEN_RCOLSIZE);}

<INITIAL,NUM,DREF>{COLON}	{/*++colons; if(colons == 2){BEGIN(DREF);}*/return(TOKEN_COLON);}

<NUM>{NEQ}                   {return(TOKEN_NE);}
<NUM>{EQ}                    {return(TOKEN_EQ);}
<NUM>{LE}                    {return(TOKEN_LE);}
<NUM>{GE}                    {return(TOKEN_GE);}
<INITIAL,NUM>{PIPE}                   {return(TOKEN_PIPE);}
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
<DREF>{RBRACKET}              {return(TOKEN_LEFT_BRACKET);}  
<DREF>{LBRACKET}              {return(TOKEN_RIGHT_BRACKET);}  
<INITIAL,NUM,DREF>#.*			{}
<ERR>[^\r\n]*           {return(TOKEN_ERR);}
<INITIAL,ERR,NUM,DREF>{EOL}	{
                          BEGIN(INITIAL);
                          //fprintf(stderr, ">>>>>>>>>>Initial!\n");
                        }
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
<INITIAL,NUM,DREF>. {xcDebug("XChecklist: Unmatched character '%s' at line %d!\n", chkltext, chkllineno);}
%%

void start_error_recovery()
{
  //printf("Starting error recovery!!!\n");
  BEGIN(ERR);
}

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

