#include "tokenizer.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>

tokenizer_t mk_tokenizer(char * filename)
{
    tokenizer_t t = (tokenizer_t)malloc(sizeof(struct tokenizer));
    t->filename = filename;
    t->line = 1;
    t->fp = safe_fopen(filename, "rb");
    if (t->fp == NULL) {
        fprintf(stderr, "cannot open input file\n");
    }

    t->c = fgetc(t->fp);
    return t;
}

token cur_tok(tokenizer_t t)
{
    return t->tok;
}

//isalpha にアンダースコアをケアさせたもの
static bool my_isalpha(int c){
    return (isalpha(c) || c == '_');
}

void set_kind(tokenizer_t t, token_kind_t kind){
    t->tok.kind = kind;
}

void set_ival(tokenizer_t t, int val){
    t->tok.ival = val;
}

//字句を読み込み種類を判別する
token next_tok(tokenizer_t t)
{
    //空白は捨てる
    while(t->c == ' ') t->c = fgetc(t->fp);

    //数字:TOK_INT_LITERAL
    if(isdigit(t->c)){
        int x = t->c - '0';
        while(isdigit(t->c = fgetc(t->fp))){
            x = x * 10 + (t->c - '0');
        }
        set_kind(t, TOK_INT_LITERAL);
        set_ival(t, x);
    }
    //ID | keyword
    else if(my_isalpha(t->c)){
        char *str = (char *)malloc(100);
        //char str[10000] = {0};
        int i = 0;
        str[i] = t->c;
        while(1){
            i++;
            int next = (t->c = fgetc(t->fp));
            if(!(my_isalpha(next) || isdigit(next))){
                break;
            }
            str[i] = t->c;
        }
        //printf("%s\n", str);
        if(strcmp(str, "break") == 0){
            set_kind(t, TOK_BREAK);
        }else if(strcmp(str, "continue") == 0){
            set_kind(t, TOK_CONTINUE);
        }else if(strcmp(str, "else") == 0){
            set_kind(t, TOK_ELSE);
        }else if(strcmp(str, "if") == 0){
            set_kind(t, TOK_IF);
        }else if(strcmp(str, "int") == 0){
            set_kind(t, TOK_INT);
        }else if(strcmp(str, "return") == 0){
            set_kind(t, TOK_RETURN);
        }else if(strcmp(str, "while") == 0){
            set_kind(t, TOK_WHILE);
        }else{
            set_kind(t, TOK_ID);
            t->tok.str = str;
            //strcpy(t->tok.str, str);
        }
    }
    //symbol
    else{
        switch(t->c){
        case '(':
            set_kind(t, TOK_LPAREN);
            break;
        case ')':
            set_kind(t, TOK_RPAREN);
            break;
        case '{':
            set_kind(t, TOK_LBRACE);
            break;
        case '}':
            set_kind(t, TOK_RBRACE);
            break;
        case '*':
            set_kind(t, TOK_MUL);
            break;
        case '+':
            set_kind(t, TOK_PLUS);
            break;
        case '-':
            set_kind(t, TOK_MINUS);
            break;
        case '/':
            set_kind(t, TOK_DIV);
            break;
        case '%':
            set_kind(t, TOK_REM);
            break;
        case ';':
            set_kind(t, TOK_SEMICOLON);
            break;
        case ',':
            set_kind(t, TOK_COMMA);
            break;
        case '\n':
            t->c = fgetc(t->fp);
            t->line++;
            next_tok(t);//set_kind(t, TOK_NEWLINE);
            fseek(t->fp, -1, 1);
            break;
        case '\t':
            //set_kind(t, TOK_TAB);
            t->c = fgetc(t->fp);
            next_tok(t);
            fseek(t->fp, -1, 1);
            break;
        case EOF:
            set_kind(t, TOK_EOF);
            break;
        case '<':
            if((t->c = fgetc(t->fp)) == '='){
                set_kind(t, TOK_LE);
            }
            //次の文字が=じゃなかったらfpを戻しておく
            else{
                fseek(t->fp, -1, 1);
                set_kind(t, TOK_LT);
            }
            break;
        case '>':
            if((t->c = fgetc(t->fp)) == '='){
                set_kind(t, TOK_GE);
            }
            //次の文字が=じゃなかったらfpを戻しておく
            else{
                fseek(t->fp, -1, 1);
                set_kind(t, TOK_GT);
            }
            break;
        case '=':
            if((t->c = fgetc(t->fp)) == '='){
                set_kind(t, TOK_EQ);
            }
            //次の文字が=じゃなかったらfpを戻しておく
            else{
                fseek(t->fp, -1, 1);
                set_kind(t, TOK_ASSIGN);
            }
            break;
        case '!':
            if((t->c = fgetc(t->fp)) == '='){
                set_kind(t, TOK_NEQ);
            }
            //次の文字が=じゃなかったらfpを戻しておく
            else{
                fseek(t->fp, -1, 1);
                set_kind(t, TOK_BANG);
            }
            break;
        default:
            syntax_error(t);
        }
        t->c = fgetc(t->fp);
    }
    return cur_tok(t);
}


#define MAKE_CASE(TYPE) case (TYPE) : return #TYPE

static const char *kindToString(token_kind_t k){
    switch(k){
    MAKE_CASE(TOK_BREAK);
    MAKE_CASE(TOK_CONTINUE);
    MAKE_CASE(TOK_ELSE);
    MAKE_CASE(TOK_IF);
    MAKE_CASE(TOK_INT);
    MAKE_CASE(TOK_RETURN);
    MAKE_CASE(TOK_WHILE);
    MAKE_CASE(TOK_ID);
    MAKE_CASE(TOK_LPAREN);
    MAKE_CASE(TOK_RPAREN);
    MAKE_CASE(TOK_LBRACE);
    MAKE_CASE(TOK_RBRACE);
    MAKE_CASE(TOK_MUL);
    MAKE_CASE(TOK_PLUS);
    MAKE_CASE(TOK_MINUS);
    MAKE_CASE(TOK_DIV);
    MAKE_CASE(TOK_REM);
    MAKE_CASE(TOK_BANG);
    MAKE_CASE(TOK_LT);
    MAKE_CASE(TOK_GT);
    MAKE_CASE(TOK_LE);
    MAKE_CASE(TOK_GE);
    MAKE_CASE(TOK_EQ);
    MAKE_CASE(TOK_NEQ);
    MAKE_CASE(TOK_SEMICOLON);
    MAKE_CASE(TOK_ASSIGN);
    MAKE_CASE(TOK_COMMA);
    default:
        return "NOTHING";
    }
}

void dump(token tok, int line){
    if(tok.kind == TOK_INT_LITERAL){
        printf("%d:TOK_INT_LITERAL (%d)\n", line, tok.ival);
    }else if(tok.kind == TOK_ID){
        printf("%d:TOK_ID (%s)\n", line, tok.str);
    }
    else{
        const char *str = kindToString(tok.kind);
        if(str != "NOTHING"){
            printf("%d:%s\n", line, kindToString(tok.kind));
        }
    }
}

void syntax_error(tokenizer_t t){
    fprintf(stderr, "syntax error at %s:%d\n", t->filename, t->line);
    exit(1);
}
