%{

  #include <stdlib.h>
  #include <string.h>
  //#include <stdbool.h>
  #include "chkl_yacc.h"
  
%}
%option noyywrap
%option nounput
%option noinput
%option nodefault
%option warn
%option yylineno

%s PAYLOAD
%x WHATEVER ERR NUM

EOL                     (\r|\n|\r\r)
STRING			[^\n\r:\[\]!=<>\|]+
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
sw_checklist            {BEGIN(PAYLOAD);return(TOKEN_CHECKLIST);}
sw_item                 {BEGIN(PAYLOAD);return(TOKEN_ITEM);}
sw_iteminfo             {BEGIN(PAYLOAD);return(TOKEN_ITEMINFO);}
sw_itemvoid             {BEGIN(WHATEVER);return(TOKEN_ITEMVOID);}
sw_show                 {BEGIN(PAYLOAD);return(TOKEN_SHOW);}
sw_rcolsize             {BEGIN(PAYLOAD);return(TOKEN_RCOLSIZE);}

<WHATEVER,PAYLOAD,NUM>{COLON}	{return(TOKEN_COLON);}

<NUM>{NEQ}                   {return(TOKEN_NE);}
<NUM>{EQ}                    {return(TOKEN_EQ);}
<NUM>{LE}                    {return(TOKEN_LE);}
<NUM>{GE}                    {return(TOKEN_GE);}
{RBRACKET}              {return(TOKEN_LEFT_BRACKET);}  
{LBRACKET}              {return(TOKEN_RIGHT_BRACKET);}  
<NUM,PAYLOAD>{PIPE}     {return(TOKEN_PIPE);}
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
#.*			{}
<ERR>[^\r\n]*           {return(TOKEN_ERR);}
<INITIAL,PAYLOAD,WHATEVER,ERR,NUM>{EOL}	{
                          BEGIN(INITIAL);
                        }
<PAYLOAD>{STRING}	{
			  chkllval.str=strdup(yytext);
//			  fprintf(stderr, "\nSTRING:'%s'\n", yytext);
			  return(TOKEN_STRING);
			}
<WHATEVER>[^:\r\n]*     {
			  chkllval.str=strdup(yytext);
//			  fprintf(stderr, "\nWhatever:'%s'\n", yytext);
			  return(TOKEN_STRING);
                        }
<PAYLOAD,INITIAL,WHATEVER,NUM>. {fprintf(stderr, "Problem: Unmatched character '%s'!\n", yytext);}
%%

void start_error_recovery()
{
  printf("Starting error recovery!!!\n");
  BEGIN(ERR);
}

void expect_number()
{
  BEGIN(NUM);
}
