#include "parse.h"
#include <stdbool.h>
#include <string.h>

void eat_it(tokenizer_t t, token_kind_t kind){
    if(cur_tok(t).kind != kind) syntax_error(t);
    next_tok(t);
}

program_t parse_program(tokenizer_t t){
    fun_def_list_t f_list = parse_fun_def_list(t);
    if(cur_tok(t).kind != TOK_EOF) syntax_error(t);
    return mk_program(t->filename, f_list);
}

fun_def_list_t parse_fun_def_list(tokenizer_t t){
    fun_def_list_t f_list = mk_fun_def_list();
    while(cur_tok(t).kind != TOK_EOF){
        fun_def_list_add(f_list, parse_fun_def(t));
    }
    return f_list;
}

fun_def_t parse_fun_def(tokenizer_t t){
    char *type = parse_type_expr(t);
    if(cur_tok(t).kind != TOK_ID) syntax_error(t);
    char *id_str = (char * )malloc(100);
    //char id_str[100];
    strcpy(id_str, cur_tok(t).str);
    next_tok(t);
    eat_it(t,TOK_LPAREN);
    var_decl_list_t v_list = parse_param_list(t);
    eat_it(t, TOK_RPAREN);
    stmt_t s = parse_stmt_compound(t);
    return mk_fun_def(t->filename, t->line, id_str, v_list, s);
}

var_decl_list_t parse_param_list(tokenizer_t t){
    var_decl_list_t v_list = mk_var_decl_list();
    if(type_token_check(t)){

        var_decl_list_add(v_list, parse_param(t));

        while(cur_tok(t).kind == TOK_COMMA){
            next_tok(t);
            var_decl_list_add(v_list, parse_param(t));
        }
    }
    return v_list;
}

var_decl_t parse_param(tokenizer_t t){
    char *type = parse_type_expr(t);
    if(cur_tok(t).kind != TOK_ID) syntax_error(t);
    char *id_str = (char *)malloc(100);
    strcpy(id_str, cur_tok(t).str);
    next_tok(t);
    return mk_var_decl(t->filename, t->line, id_str);
}

char *parse_type_expr(tokenizer_t t){
    if(cur_tok(t).kind == TOK_INT){
        next_tok(t);
        return "int";
    }
    syntax_error(t);
}

bool type_token_check(tokenizer_t t){
    return cur_tok(t).kind == TOK_INT; //型が増えたらここにOR記述
}

var_decl_t parse_var_decl(tokenizer_t t){
    char *type = parse_type_expr(t);
    if(cur_tok(t).kind != TOK_ID) syntax_error(t);
    char *v_str = (char *)malloc(100);
    strcpy(v_str, cur_tok(t).str);
    //char * v_str = cur_tok(t).str;
    next_tok(t);
    eat_it(t, TOK_SEMICOLON);
    return mk_var_decl(t->filename, t->line, v_str);
}

stmt_t parse_stmt(tokenizer_t t){
    switch(cur_tok(t).kind){
    case TOK_SEMICOLON:
        return parse_stmt_empty(t);
    case TOK_CONTINUE:
        return parse_stmt_continue(t);
    case TOK_BREAK:
        return parse_stmt_break(t);
    case TOK_RETURN:
        return parse_stmt_return(t);
    case TOK_LBRACE:
        return parse_stmt_compound(t);
    case TOK_IF:
        return parse_stmt_if(t);
    case TOK_WHILE:
        return parse_stmt_while(t);
    default:
        return parse_stmt_expr(t);
    }
}

stmt_t parse_stmt_empty(tokenizer_t t){
    eat_it(t, TOK_SEMICOLON);
    return mk_stmt_empty(t->filename, t->line);
}

stmt_t parse_stmt_continue(tokenizer_t t){
    eat_it(t, TOK_CONTINUE);
    eat_it(t, TOK_SEMICOLON);
    return mk_stmt_continue(t->filename, t->line);
}

stmt_t parse_stmt_break(tokenizer_t t){
    eat_it(t, TOK_BREAK);
    eat_it(t, TOK_SEMICOLON);
    return mk_stmt_break(t->filename, t->line);
}

stmt_t parse_stmt_return(tokenizer_t t){
    eat_it(t, TOK_RETURN);
    if(cur_tok(t).kind != TOK_SEMICOLON){
        expr_t e = parse_expr(t);
        eat_it(t, TOK_SEMICOLON);
        return mk_stmt_return(t->filename, t->line, e);
    }
    return mk_stmt_return(t->filename, t->line, NULL);
}

stmt_t parse_stmt_compound(tokenizer_t t){
    eat_it(t, TOK_LBRACE);
    var_decl_list_t v_list = mk_var_decl_list();
    while(type_token_check(t)){
        var_decl_list_add(v_list, parse_var_decl(t));
    }
    stmt_list_t s_list = mk_stmt_list();
    while(cur_tok(t).kind != TOK_RBRACE){
        stmt_list_add(s_list, parse_stmt(t));
    }
    eat_it(t, TOK_RBRACE);
    return mk_stmt_compound(t->filename, t->line, v_list, s_list);
}

stmt_t parse_stmt_if(tokenizer_t t){
    eat_it(t, TOK_IF);
    eat_it(t, TOK_LPAREN);
    expr_t e = parse_expr(t);
    eat_it(t, TOK_RPAREN);
    stmt_t body = parse_stmt(t);

    if(cur_tok(t).kind == TOK_ELSE){
        eat_it(t, TOK_ELSE);
        return mk_stmt_if(t->filename, t->line, e, body, parse_stmt(t));
    }else{
        return mk_stmt_if(t->filename, t->line, e, body, NULL);
    }
}

stmt_t parse_stmt_while(tokenizer_t t){
    // while
    eat_it(t, TOK_WHILE);
    // (
    eat_it(t, TOK_LPAREN);
    expr_t e = parse_expr(t);
    // )
    eat_it(t, TOK_RPAREN);
    stmt_t body = parse_stmt(t);
    return mk_stmt_while(t->filename, t->line, e, body);
}

stmt_t parse_stmt_expr(tokenizer_t t){
    expr_t e = parse_expr(t);
    eat_it(t, TOK_SEMICOLON);
    return mk_stmt_expr(t->filename, t->line, e);
}

expr_t parse_expr(tokenizer_t t){
    expr_t e0 = parse_equality_expr(t);
    if(cur_tok(t).kind != TOK_ASSIGN){
        return e0;
    }
    else{
        next_tok(t);
        expr_t e1 = parse_expr(t);
        return mk_expr_bin_op(t->filename, t->line, op_kind_assign, e0, e1);
    }
}

static expr_t parse_recursive_equality_expr(tokenizer_t t, expr_t e){
    switch(cur_tok(t).kind){
    case TOK_EQ:
        next_tok(t);
        return mk_expr_bin_op(t->filename, t->line, op_kind_eq, e, parse_recursive_equality_expr(t, parse_relational_expr(t)));
    case TOK_NEQ:
        next_tok(t);
        return mk_expr_bin_op(t->filename, t->line, op_kind_neq, e, parse_recursive_equality_expr(t, parse_relational_expr(t)));
    default:
        return e;
    }
}

expr_t parse_equality_expr(tokenizer_t t){
    return parse_recursive_equality_expr(t, parse_relational_expr(t));
}

static expr_t parse_recursive_relational_expr(tokenizer_t t, expr_t e){
    switch(cur_tok(t).kind){
    case TOK_LT:
        next_tok(t);
        return mk_expr_bin_op(t->filename, t->line, op_kind_lt, e, parse_recursive_relational_expr(t, parse_additive_expr(t)));
    case TOK_GT:
        next_tok(t);
        return mk_expr_bin_op(t->filename, t->line, op_kind_gt, e, parse_recursive_relational_expr(t, parse_additive_expr(t)));
    case TOK_LE:
        next_tok(t);
        return mk_expr_bin_op(t->filename, t->line, op_kind_le, e, parse_recursive_relational_expr(t, parse_additive_expr(t)));
    case TOK_GE:
        next_tok(t);
        return mk_expr_bin_op(t->filename, t->line, op_kind_ge, e, parse_recursive_relational_expr(t, parse_additive_expr(t)));
    default:
        return e;
    }
}

expr_t parse_relational_expr(tokenizer_t t){
    return parse_recursive_relational_expr(t, parse_additive_expr(t));
}

static expr_t parse_recursive_additive_expr(tokenizer_t t, expr_t e){
    switch(cur_tok(t).kind){
    case TOK_PLUS:
        next_tok(t);
        return mk_expr_bin_op(t->filename, t->line, op_kind_bin_plus, e, parse_recursive_additive_expr(t, parse_multiplicative_expr(t)));
    case TOK_MINUS:
        next_tok(t);
        return mk_expr_bin_op(t->filename, t->line, op_kind_bin_minus, e, parse_recursive_additive_expr(t, parse_multiplicative_expr(t)));
    default:
        return e;
    }
}

expr_t parse_additive_expr(tokenizer_t t){
    return parse_recursive_additive_expr(t, parse_multiplicative_expr(t));
}

static expr_t parse_recursive_multiplicative_expr(tokenizer_t t, expr_t e){
    switch(cur_tok(t).kind){
    case TOK_MUL:
        next_tok(t);
        return mk_expr_bin_op(t->filename, t->line, op_kind_mult, e, parse_recursive_multiplicative_expr(t, parse_unary_expr(t)));
    case TOK_DIV:
        next_tok(t);
        return mk_expr_bin_op(t->filename, t->line, op_kind_div, e, parse_recursive_multiplicative_expr(t, parse_unary_expr(t)));
    case TOK_REM:
        next_tok(t);
        return mk_expr_bin_op(t->filename, t->line, op_kind_rem, e, parse_recursive_multiplicative_expr(t, parse_unary_expr(t)));
    default:
        return e;
    }
}

expr_t parse_multiplicative_expr(tokenizer_t t){
    return parse_recursive_multiplicative_expr(t, parse_unary_expr(t));
}

expr_t parse_unary_expr(tokenizer_t t){
    char *s;
    switch(cur_tok(t).kind){
    case TOK_INT_LITERAL:
        s = (char *)malloc(100);
        //数値の文字列表現を得る
        sprintf(s, "%d", cur_tok(t).ival);
        next_tok(t);
        return mk_expr_int_literal(t->filename, t->line, s);
    case TOK_ID:
        s = (char *)malloc(100);
        strcpy(s, cur_tok(t).str);
        next_tok(t);
        if(cur_tok(t).kind == TOK_LPAREN){
            next_tok(t);
            return mk_expr_call(t->filename, t->line, s, parse_argument_expr_list(t));
        }
        return mk_expr_id(t->filename, t->line, s);
    case TOK_LPAREN:
        next_tok(t);
        expr_t e = parse_expr(t);
        eat_it(t, TOK_RPAREN);
        return mk_expr_paren(t->filename, t->line, e);
    case TOK_PLUS:
        next_tok(t);
        return mk_expr_un_op(t->filename, t->line, op_kind_un_plus, parse_unary_expr(t));
    case TOK_MINUS:
        next_tok(t);
        return mk_expr_un_op(t->filename, t->line, op_kind_un_minus, parse_unary_expr(t));
    case TOK_BANG:
        next_tok(t);
        return mk_expr_un_op(t->filename, t->line, op_kind_logneg, parse_unary_expr(t));
    default:
        syntax_error(t);
    }
}

expr_list_t parse_argument_expr_list(tokenizer_t t){
    expr_list_t argument_list = mk_expr_list();
    if(cur_tok(t).kind != TOK_RPAREN){ //これはargument list が括弧の中にあるという仮定をおいてしまっているので良いのかあやしい
        expr_list_add(argument_list, parse_expr(t));
        while(1){
            if(cur_tok(t).kind == TOK_RPAREN) break;
            eat_it(t, TOK_COMMA);
            expr_list_add(argument_list, parse_expr(t));
        }
    }
    next_tok(t);
    return argument_list;
}



