#include <stdio.h>
#include <stdlib.h>
#include "tokenizer.h"
#include "syntree.h"
#include "cogen.h"

#if 0
//tokenizer
int main(int argc, char ** argv)
{
    tokenizer_t t = mk_tokenizer(argv[1]);
    int line = 0; //行番号
    while (cur_tok(t).kind != TOK_EOF) {
    int token_index = 0;
        line++;
        while (1) {
            dump(next_tok(t), line);
            if(cur_tok(t).kind == TOK_EOF ) break;
        }
    }
    return 0;
}
#endif


#if 1
//構文解析
int main(int argc, char **argv){
    tokenizer_t t = mk_tokenizer(argv[1]);
    next_tok(t);

    program_t p = parse_program(t);
    //pr_program(stdout, p);

    cogen_program(stdout, p, argv[1]);

    //stmt_t s = parse_stmt_while(t);
    //pr_stmt(stdout, s);
    //expr_t e = parse_expr(t);
    //pr_expr(stdout, e);
}
#endif

