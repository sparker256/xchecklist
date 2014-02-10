%{

  #include <stdlib.h>
  #include <string.h>
  //#include <stdbool.h>
  #include "chkl_yacc.h"
  #include "utils.h"
  
  static bool isVoid = false;

%}
%option noyywrap
%option nounput
%option noinput
%option nodefault
%option warn
%option yylineno

%x NUM DREF VOID

EOL                     (\r|\n)
EOL2                     (\n\r|\r\n)
STRING			        [^\n\r:|]+
VOIDSTRING		        [^\n\r]+
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
sw_checklist            {isVoid = false; return(TOKEN_CHECKLIST);}
sw_item                 {isVoid = false; return(TOKEN_ITEM);}
sw_iteminfo             {isVoid = false; return(TOKEN_ITEMINFO);}
sw_itemvoid             {isVoid = true; return(TOKEN_ITEMVOID);}
sw_show                 {isVoid = false; return(TOKEN_SHOW);}
sw_rcolsize             {isVoid = false; return(TOKEN_RCOLSIZE);}

<INITIAL,NUM,DREF>{COLON}	{if(isVoid){BEGIN(VOID);} return(TOKEN_COLON);}

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
<INITIAL,NUM,DREF,VOID>#.*			{}
<INITIAL,NUM,DREF,VOID>{EOL2}	{
                          BEGIN(INITIAL);
                          isVoid = false;
                          //fprintf(stderr, ">>>>>>>>>>Initial!\n");
                        }
<INITIAL,NUM,DREF,VOID>{EOL}	{
                          BEGIN(INITIAL);
                          isVoid = false;
                          //fprintf(stderr, ">>>>>>>>>>Initial!\n");
                        }
<INITIAL,NUM,DREF,VOID>[[:blank:]]      /*eat whitespaces*/          
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
<INITIAL,NUM,DREF,VOID>. {
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

