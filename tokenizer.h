#pragma once 
#include <stdlib.h>
#include <stdio.h>
#include "syntree.h"

typedef enum {
    //keyword
    TOK_BREAK,
    TOK_CONTINUE,
    TOK_ELSE,
    TOK_IF,
    TOK_INT,
    TOK_RETURN,
    TOK_WHILE,
    //identifier
    TOK_ID,
    //int-literal
    TOK_INT_LITERAL,
    //symbol
    TOK_LPAREN, // (
    TOK_RPAREN, // )
    TOK_LBRACE, // {
    TOK_RBRACE, // }
    TOK_MUL,    // *
    TOK_PLUS,   // +
    TOK_MINUS,  // -
    TOK_DIV,    // /
    TOK_REM,    // %
    TOK_BANG,   // !
    TOK_LT,     // <
    TOK_GT,     // >
    TOK_LE,     // <=
    TOK_GE,     // >=
    TOK_EQ,     // ==
    TOK_NEQ,    // !=
    TOK_SEMICOLON,  // ;
    TOK_ASSIGN, // =
    TOK_COMMA,  // ,
    TOK_NEWLINE,// 改行
    TOK_EOF,    // EOF
    TOK_TAB     //TAB
} token_kind_t;


typedef struct token
{
  token_kind_t kind;
  int ival;
  //char str[10000];
  char *str;
} token;

typedef struct tokenizer
{
  token tok;		      /* 最後に読んだ字句 */
  int c;		      /* 最後に読んだ文字(=tokの一文字後ろ) */
  FILE *fp;
  char *filename;       //ファイル名
  int line;             //行番号
} * tokenizer_t;

tokenizer_t mk_tokenizer(char * filename);
token next_tok(tokenizer_t t);
token next_tok_test(tokenizer_t t, expr_t e);
token cur_tok(tokenizer_t t);
void dump(token tok, int line);
void syntax_error(tokenizer_t t);
