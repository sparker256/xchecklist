%{
  #include <stdio.h>
  #include <string.h>
  #include <chkl_parser.h>
  extern FILE* yyin;
  int yylex (void);
  void yyerror (char const *);

  class checklist;
  class checklist_binder;
  checklist *current_checklist = NULL;
  extern void start_error_recovery();
  extern void expect_number();
  extern void expect_dataref();
  extern void expect_nothing();
  
  checklist *empty_checklist(class checklist_binder **cb)
  {
    if(*cb == NULL){
      *cb = new checklist_binder;
    }
    class checklist *cl = new class checklist("(NO NAME)");
    (*cb)->add_checklist(cl);
    return cl;
  }
  
%}

%debug
%error-verbose

%union {
  char *str;
  class checklist *chkl;
  class dataref_name *dref_name;
  class dataref_t *dref;
  class item_label *lbl;
  class checklist_item *item;
  class number *num;
  int *op;
}

%token TOKEN_CHECKLIST
%token TOKEN_ITEM
%token TOKEN_ITEMINFO
%token TOKEN_ITEMVOID
%token TOKEN_SHOW
%token TOKEN_RCOLSIZE
%token TOKEN_COLON
%token TOKEN_PIPE
%token TOKEN_NE
%token TOKEN_LT
%token TOKEN_GT
%token TOKEN_LE
%token TOKEN_GE
%token TOKEN_COMMENT
%token TOKEN_LEFT_BRACKET
%token TOKEN_RIGHT_BRACKET
%token TOKEN_LEFT_PARENTHESIS
%token TOKEN_RIGHT_PARENTHESIS
%token TOKEN_AND
%token TOKEN_OR
%token TOKEN_EQ
%token TOKEN_EOF
%token <str> TOKEN_STRING TOKEN_FRAC TOKEN_EXPONENT TOKEN_NUMBER
%token TOKEN_REMARK
%token TOKEN_ERR

%type <chkl> checklist;
%type <dref_name> dataref_name;
%type <dref> dataref dataref_expr dataref_term dataref_prim;
%type <op> operation;
%type <lbl> spec_string;
%type <op> colsize;
%type <item> show item_void item_info item item_remark; 
%type <num> number;

%%
input:		/* empty */
      		| input line{
      		    expect_nothing();
      		  }
;

line:		checklist{
                    if(binder == NULL){
                      binder = new checklist_binder;
                    }
                    current_checklist = $1;
                    binder->add_checklist(current_checklist);
                  }
		| item{
		    if(current_checklist == NULL){
		      current_checklist = empty_checklist(&binder);
		    }
                    current_checklist->add_item($1);
                  }
		| item_info{
		    if(current_checklist == NULL){
		      current_checklist = empty_checklist(&binder);
		    }
                    current_checklist->add_item($1);
                  }
		| item_void {
		    if(current_checklist == NULL){
		      current_checklist = empty_checklist(&binder);
		    }
                    current_checklist->add_item($1);
                  }
		| item_remark {
		    if(current_checklist == NULL){
		      current_checklist = empty_checklist(&binder);
		    }
                    current_checklist->add_item($1);
                  }
		| show {
		    if(current_checklist == NULL){
		      current_checklist = empty_checklist(&binder);
		    }
                    current_checklist->add_item($1);
                  }
		| colsize {
		    if(current_checklist == NULL){
		      current_checklist = empty_checklist(&binder);
		    }
                    current_checklist->set_width(*$1);
                    delete($1);
                  }
		| error {
		    yyclearin;
		    yyerrok;
		  }
;

checklist:	TOKEN_CHECKLIST TOKEN_COLON TOKEN_STRING {
                    $$ = new class checklist($3);
                    printf("New checklist def '%s'!\n", $3);
                    free($3);
                  }
		| TOKEN_CHECKLIST TOKEN_COLON TOKEN_STRING 
		    TOKEN_COLON TOKEN_STRING {
                    $$ = new class checklist($3, $5);
                    printf("New checklist def1 '%s'!\n", $3);
                    free($3);
                    free($5);
                  }
;

item:		TOKEN_ITEM TOKEN_COLON spec_string {
                    $$ = new chk_item($3, NULL, true);
                  }
		| TOKEN_ITEM TOKEN_COLON spec_string TOKEN_COLON{
                    $$ = new chk_item($3, NULL, true);
                  }
		| TOKEN_ITEM TOKEN_COLON spec_string TOKEN_COLON dataref_expr{
                    $$ = new chk_item($3, $5, true);
                  }
;
item_info:	TOKEN_ITEMINFO TOKEN_COLON spec_string {
                    $$ = new chk_item($3, NULL, false);
                  }
		| TOKEN_ITEMINFO TOKEN_COLON spec_string TOKEN_COLON dataref_expr{
                    $$ = new chk_item($3, $5, false);
                  }
;
item_void:      TOKEN_ITEMVOID TOKEN_COLON TOKEN_STRING{
                    $$ = new void_item($3);
                    free($3);
                  }
                | TOKEN_ITEMVOID TOKEN_COLON{
                    $$ = new void_item("");
                  }
;
item_remark:    TOKEN_REMARK TOKEN_COLON TOKEN_STRING{
                    $$ = new remark_item($3);
                    free($3);
                  }
;
show:		TOKEN_SHOW TOKEN_COLON dataref_expr{
                    $$ = new show_item($3);
                  }
;
colsize:	TOKEN_RCOLSIZE TOKEN_COLON TOKEN_STRING{
                    $$ = new int(atoi($3));
                    free($3);
                  }
;
spec_string:    TOKEN_STRING{
                    $$ = new item_label($1);
                    free($1);
                    expect_dataref();
                  }
		| TOKEN_STRING TOKEN_PIPE{
                    $$ = new item_label($1);
                    free($1);
                    expect_dataref();
                  }
		| TOKEN_STRING TOKEN_PIPE TOKEN_STRING{
                    $$ = new item_label($1, $3);
                    free($1);
                    free($3);
                    expect_dataref();
                  }
;
dataref_expr:   dataref_expr TOKEN_OR dataref_term {
                  $$ = new dataref_op($1, XC_OR, $3);
                }
                | dataref_term
;
dataref_term:   dataref_term TOKEN_AND dataref_prim {
                  $$ = new dataref_op($1, XC_AND, $3);
                }
                | dataref_prim
;
dataref_prim:   dataref
                | TOKEN_LEFT_PARENTHESIS dataref_expr TOKEN_RIGHT_PARENTHESIS {
                    $$ = $2;
                }
;
dataref:	dataref_name TOKEN_COLON operation number {
                    $$ = new dataref_dsc($1, (operation_t*)$3, $4);
                    delete $3;expect_dataref();
                  }
		| dataref_name TOKEN_COLON number {
                    $$ = new dataref_dsc($1, $3);expect_dataref();
                  }
                | dataref_name TOKEN_COLON number TOKEN_PIPE number {
                    $$ = new dataref_dsc($1, $3, $5);expect_dataref();
                  }
                | dataref_name TOKEN_COLON number TOKEN_COLON number {
                    $$ = new dataref_dsc($1, $3, $5, false);expect_dataref();
                  }
;
dataref_name:   TOKEN_STRING {
                    $$ = new dataref_name($1);
                    free($1);
                    expect_number();
                  }
		| TOKEN_STRING TOKEN_LEFT_BRACKET TOKEN_STRING TOKEN_RIGHT_BRACKET{
                    $$ = new dataref_name($1, $3);
                    free($1);
                    free($3);
                    expect_number();
                  }
;

operation:      TOKEN_NE {$$ = (int *)new operation_t(XC_NOT);}
                | TOKEN_LT {$$ = (int *)new operation_t(XC_LT);}
                | TOKEN_GT {$$ = (int *)new operation_t(XC_GT);}
                | TOKEN_LE {$$ = (int *)new operation_t(XC_LE);}
                | TOKEN_GE {$$ = (int *)new operation_t(XC_GE);}
                | TOKEN_EQ {$$ = (int *)new operation_t(XC_EQ);}
;

number:         TOKEN_NUMBER 
                  {$$ = new number($1, "", ""); free($1);}
                | TOKEN_NUMBER TOKEN_FRAC
                  {$$ = new number($1, $2, ""); free($1); free($2);}
                | TOKEN_NUMBER TOKEN_FRAC TOKEN_EXPONENT
                  {$$ = new number($1, $2, $3); free($1); free($2); free($3);}
                | TOKEN_NUMBER TOKEN_EXPONENT
                  {$$ = new number($1, "", $2); free($1); free($2);}
                | TOKEN_FRAC TOKEN_EXPONENT
                  {$$ = new number("", $1, $2); free($1); free($2);}
                | TOKEN_FRAC
                  {$$ = new number("", $1, ""); free($1);}
;

%%

