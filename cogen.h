#pragma once
#include "syntree.h"
#include "environment.h"

int cogen_program_header(FILE *fp, char *filename);
int cogen_program_trailer(FILE *fp, char *filename);

int cogen_program(FILE *fp, program_t p, char *filename);
int cogen_fun_def(FILE *fp, fun_def_t fd, char *filename, int index, int *p_label_sz);

int cogen_stmt_kind_empty(FILE *fp, stmt_t s, environment_t env);		/* 空文 ; */
int cogen_stmt_kind_continue(FILE *fp, stmt_t s, environment_t env);		/* continue ; */
int cogen_stmt_kind_break(FILE *fp, stmt_t s, environment_t env);		/* break ; */
int cogen_stmt_kind_return(FILE *fp, stmt_t s, environment_t env);		/* return 式 ; */
int cogen_stmt_kind_expr(FILE *fp, stmt_t s, environment_t env);		/* 式文 ; (f(x, y); など) */
int cogen_stmt_kind_compound(FILE *fp, stmt_t s, environment_t env);		/* 複合文 { ... } */
int cogen_stmt_kind_if(FILE *fp, stmt_t s, environment_t env);			/* if文 */
int cogen_stmt_kind_while(FILE *fp, stmt_t s, environment_t env);		/* while文 */


char *cogen_expr(FILE *fp, expr_t e, environment_t env);
char *cogen_expr_kind_int_literal(FILE *fp, expr_t e, environment_t env);	/* 整数リテラル (0, 1, 2, ...) */
char *cogen_expr_kind_id(FILE *fp, expr_t e, environment_t env);			/* 変数 (x, y, foo, bar, ...) */
char *cogen_expr_kind_paren(FILE *fp, expr_t e, environment_t env);		/* 括弧 ( 式 ) */
char *cogen_expr_kind_app(FILE *fp, expr_t e, environment_t env);		/* 関数呼び出し(f(1, 2)など.*/

char *op_kind_to_jmp_str_for_if(op_kind_t op);
char *op_kind_to_jmp_str_for_while(op_kind_t op);
