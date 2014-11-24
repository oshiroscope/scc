#pragma once
#include "tokenizer.h"
#include "syntree.h"
#include <stdbool.h>



//program を解析する
program_t parse_program(tokenizer_t t);

//function definition listを解析する
fun_def_list_t parse_fun_def_list(tokenizer_t t);

//function definitionを解析する
fun_def_t parse_fun_def(tokenizer_t t);

//parameter list を解析する
var_decl_list_t parse_param_list(tokenizer_t t);

//parameter を解析する
var_decl_t parse_param(tokenizer_t t);

//type expression を解析する
char *parse_type_expr(tokenizer_t t);

//現在のtokenがtype expressionか調べる
bool type_token_check(tokenizer_t t);

//variable declarationを解析する
var_decl_t parse_var_decl(tokenizer_t t);

//statement を解析する
stmt_t parse_stmt(tokenizer_t t);
stmt_t parse_stmt_empty(tokenizer_t t);
stmt_t parse_stmt_continue(tokenizer_t t);
stmt_t parse_stmt_break(tokenizer_t t);
stmt_t parse_stmt_return(tokenizer_t t);
stmt_t parse_stmt_compound(tokenizer_t t);
stmt_t parse_stmt_if(tokenizer_t t);
stmt_t parse_stmt_while(tokenizer_t t);
stmt_t parse_stmt_expr(tokenizer_t t);

//expression を解析する
expr_t parse_expr(tokenizer_t t);
expr_t parse_equality_expr(tokenizer_t t);
expr_t parse_relational_expr(tokenizer_t t);
expr_t parse_additive_expr(tokenizer_t t);
expr_t parse_multiplicative_expr(tokenizer_t t);
expr_t parse_unary_expr(tokenizer_t);

//argument listを解析する
expr_list_t parse_argument_expr_list(tokenizer_t t);

