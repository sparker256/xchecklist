%{
  #include <stdio.h>
  #include <string.h>
  #include "chkl_parser.h"
  extern FILE* yyin;
  int yylex (void);
  void yyerror (char const *);

  class checklist;
  class checklist_binder;
  extern void start_error_recovery();

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
  class coloured_string *c_str;
  class checklist *chkl;
  class dataref_name *dref_name;
  class dataref_t *dref;
  class item_label *lbl;
  class checklist_item *item;
  class value *val;
  std::vector<class value *> *plist;
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
%token TOKEN_POS_DIF
%token TOKEN_NEG_DIF
%token TOKEN_ABS_DIF
%token TOKEN_COMMENT
%token TOKEN_LEFT_BRACKET
%token TOKEN_RIGHT_BRACKET
%token TOKEN_LEFT_PARENTHESIS
%token TOKEN_RIGHT_PARENTHESIS
%token TOKEN_TO_DOUBLE
%token TOKEN_TO_FLOAT
%token TOKEN_TO_INT
%token TOKEN_COMA
%token <str>TOKEN_DREF
%token TOKEN_POW
%token TOKEN_PLUS
%token TOKEN_MINUS
%token TOKEN_MUL
%token TOKEN_DIV
%token TOKEN_AND
%token TOKEN_OR
%token TOKEN_EQ
%token TOKEN_EOF
%token <str> TOKEN_STRING TOKEN_NUMBER
%token TOKEN_REMARK
%token TOKEN_CONTINUE
%token TOKEN_SILENCE
%token TOKEN_ERR
%token TOKEN_COLOUR_DEF
%token <str>TOKEN_COLOUR_NAME
%token TOKEN_BACKSLASH

%type <chkl> checklist;
%type <dref> dataref dataref_expr dataref_expr_both dataref_term dataref_prim rel_primary rel_term rel_expr;
%type <op> rel_operation;
%type <lbl> spec_string;
%type <op> colsize;
%type <item> show item_void item_info item item_remark;
%type <val> dataref_name number primary p_term term expression;
%type <plist> param_list
%type <c_str> coloured_string coloured_string_element;
%%
input:                /* empty */
                      | input line
;

line:                checklist{
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
                | continue
                | TOKEN_COLOUR_DEF TOKEN_COLON TOKEN_STRING TOKEN_COLON 
                  TOKEN_NUMBER TOKEN_COMA TOKEN_NUMBER TOKEN_COMA TOKEN_NUMBER {
                    p.add_colour($3, $5, $7, $9);
                    free($3);
                    free($5);
                    free($7);
                    free($9);
                  }
                | error {
                    yyclearin;
                    yyerrok;
                  }
;
checklist:        TOKEN_CHECKLIST TOKEN_COLON TOKEN_STRING {
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

item:           TOKEN_ITEM TOKEN_COLON spec_string {
                    $$ = new chk_item($3, NULL, true);
                  }
                | TOKEN_ITEM TOKEN_COLON spec_string TOKEN_COLON{
                    $$ = new chk_item($3, NULL, true);
                  }
                | TOKEN_ITEM TOKEN_COLON spec_string TOKEN_COLON dataref_expr_both{
                    $$ = new chk_item($3, $5, true);
                  }
                | TOKEN_SILENCE TOKEN_ITEM TOKEN_COLON spec_string {
                    $$ = new chk_item($4, NULL, true, true);
                  }
                | TOKEN_SILENCE TOKEN_ITEM TOKEN_COLON spec_string TOKEN_COLON{
                    $$ = new chk_item($4, NULL, true, true);
                  }
                | TOKEN_SILENCE TOKEN_ITEM TOKEN_COLON spec_string TOKEN_COLON dataref_expr_both{
                    $$ = new chk_item($4, $6, true, true);
                  }
;
item_info:        TOKEN_ITEMINFO TOKEN_COLON spec_string {
                    $$ = new chk_item($3, NULL, true);
                  }
                | TOKEN_ITEMINFO TOKEN_COLON spec_string TOKEN_COLON dataref_expr_both{
                    $$ = new chk_item($3, $5, false);
                  }
;
item_void:      TOKEN_ITEMVOID TOKEN_COLON coloured_string TOKEN_PIPE coloured_string{
                    $3->resolve_colours();
                    $5->resolve_colours();
                    $$ = new void_item($3, $5);
                  }
                | TOKEN_ITEMVOID TOKEN_COLON coloured_string TOKEN_PIPE{
                    $3->resolve_colours();
                    $$ = new void_item($3);
                  }
                | TOKEN_ITEMVOID TOKEN_COLON coloured_string{
                    $3->resolve_colours();
                    $$ = new void_item($3);
                  }
                | TOKEN_ITEMVOID TOKEN_COLON{
                    $$ = new void_item(new coloured_string(""));
                  }
;
item_remark:    TOKEN_REMARK TOKEN_COLON coloured_string{
                    $3->resolve_colours();
                    $$ = new remark_item($3);
                  }
;
show:                TOKEN_SHOW TOKEN_COLON dataref_expr_both{
                    $$ = new show_item($3);
                  }
;
continue:       TOKEN_CONTINUE TOKEN_COLON TOKEN_STRING TOKEN_COLON dataref_expr_both{
                    current_checklist->add_continue_flag($3, $5);
                    free($3);
                  }
                | TOKEN_CONTINUE TOKEN_COLON TOKEN_STRING{
                    current_checklist->add_continue_flag($3);
                    free($3);
                  }
                | TOKEN_CONTINUE {
                    current_checklist->add_continue_flag();
                  }
;
colsize:        TOKEN_RCOLSIZE TOKEN_COLON TOKEN_STRING{
                    $$ = new int(atoi($3));
                    free($3);
                  }
;
spec_string:    coloured_string{
                    $1->resolve_colours();
                    $$ = new item_label($1);
                  }
                | coloured_string TOKEN_PIPE{
                    $1->resolve_colours();
                    $$ = new item_label($1);
                  }
                | coloured_string TOKEN_PIPE coloured_string{
                    $1->resolve_colours();
                    $3->resolve_colours();
                    $$ = new item_label($1, $3);
                  }
                | coloured_string TOKEN_PIPE coloured_string TOKEN_PIPE coloured_string TOKEN_PIPE coloured_string{
                    $1->resolve_colours();
                    $3->resolve_colours();
                    $5->resolve_colours();
                    $7->resolve_colours();
                    $$ = new item_label($1, $3, $5, $7);
                  }
;
coloured_string : coloured_string coloured_string_element {
                    $2->append($1);
                    delete($2);
                    $$ = $1;
                  }
                | coloured_string_element
;
coloured_string_element: TOKEN_COLOUR_NAME TOKEN_STRING {
                           std::string *tmp = new std::string($1);
                           $$ = new coloured_string($2, tmp);
                           delete(tmp);
                           free($1);
                           free($2);
                         }
                       | TOKEN_BACKSLASH TOKEN_STRING {
                           std::string *tmp = new std::string("");
                           $$ = new coloured_string($2, tmp);
                           delete(tmp);
                           free($2);
                         }
                       | TOKEN_STRING {
                           $$ = new coloured_string($1);
                           free($1);
                         }
;
dataref_expr_both: dataref_expr
                |  TOKEN_COLON rel_expr {
                  $$ = $2;
                }
;
rel_expr:       rel_expr TOKEN_OR rel_term {
                  $$ = new dataref_op($1, XC_OR, $3);
                }
                | rel_term
;
rel_term:       rel_term TOKEN_AND rel_primary {
                  $$ = new dataref_op($1, XC_AND, $3);
                }
                | rel_primary
;
rel_primary:    TOKEN_LEFT_PARENTHESIS expression rel_operation expression TOKEN_RIGHT_PARENTHESIS {
                    $$ = new dataref_dsc($2, (operation_t*)$3, $4);
                    delete $3;
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
dataref:        dataref_name TOKEN_COLON rel_operation expression {
                    $$ = new dataref_dsc($1, (operation_t*)$3, $4);
                    delete $3;
                  }
                | dataref_name TOKEN_COLON expression {
                    $$ = new dataref_dsc($1, $3);
                  }
                | dataref_name TOKEN_COLON expression TOKEN_PIPE expression {
                    $$ = new dataref_dsc($1, $3, $5);
                  }
                | dataref_name TOKEN_COLON expression TOKEN_COLON expression {
                    $$ = new dataref_dsc($1, $3, $5, false);
                  }
;
dataref_name:   TOKEN_STRING {
                    $$ = new dataref_name($1);
                    free($1);
                  }
                | TOKEN_NUMBER {
                    $$ = new number($1, "", "");
                    free($1);
                  }
                | TOKEN_STRING TOKEN_LEFT_BRACKET TOKEN_NUMBER TOKEN_RIGHT_BRACKET{
                    $$ = new dataref_name($1, $3);
                    free($1);
                    free($3);
                  }
;

rel_operation:  TOKEN_NE {$$ = (int *)new operation_t(XC_NOT);}
                | TOKEN_LT {$$ = (int *)new operation_t(XC_LT);}
                | TOKEN_GT {$$ = (int *)new operation_t(XC_GT);}
                | TOKEN_LE {$$ = (int *)new operation_t(XC_LE);}
                | TOKEN_GE {$$ = (int *)new operation_t(XC_GE);}
                | TOKEN_EQ {$$ = (int *)new operation_t(XC_EQ);}
                | TOKEN_POS_DIF {$$ = (int *)new operation_t(XC_POS_DIF);}
                | TOKEN_NEG_DIF {$$ = (int *)new operation_t(XC_NEG_DIF);}
                | TOKEN_ABS_DIF {$$ = (int *)new operation_t(XC_ABS_DIF);}
;

expression:     expression TOKEN_PLUS term
                  {$$ = new arith_op($1, '+', $3);}
                | expression TOKEN_MINUS term
                  {$$ = new arith_op($1, '-', $3);}
                | term

term:           term TOKEN_MUL p_term
                  {$$ = new arith_op($1, '*', $3);}
                | term TOKEN_DIV p_term
                  {$$ = new arith_op($1, '/', $3);}
                | p_term

p_term:         p_term TOKEN_POW primary
                  {$$ = new arith_op($1, '^', $3);}
                | primary


primary:        number
                | TOKEN_DREF
                  {
                    $$ = new dataref_name($1);
                    free($1);
                  }
                | TOKEN_LEFT_PARENTHESIS expression TOKEN_RIGHT_PARENTHESIS
                  {$$ = $2;}
                | TOKEN_STRING TOKEN_LEFT_PARENTHESIS param_list TOKEN_RIGHT_PARENTHESIS
                  {
                    $$ = new procedure($1, $3);
                    free($1);
                  }
                | TOKEN_MINUS primary
                  {
                    $$ = new arith_op(new number("-1", "", ""), '*', $2);
                  }
                | TOKEN_PLUS primary
                  {$$ = $2;}
                | TOKEN_TO_DOUBLE primary
                  {$2->set_type(TYPE_DOUBLE); $$ = $2;}
                | TOKEN_TO_FLOAT primary
                  {$2->set_type(TYPE_FLOAT); $$ = $2;}
                | TOKEN_TO_INT primary
                  {$2->set_type(TYPE_INT); $$ = $2;}
                

number:         TOKEN_NUMBER
                  {$$ = new number($1, "", ""); free($1);}
;

param_list:    param_list TOKEN_COMA expression
                 {$$ = $1; $$->push_back($3);}
               | expression
                 {$$ = new std::vector<class value *>(); $$->push_back($1);}
%%

