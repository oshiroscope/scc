#include "list.h"
#include "cogen.h"
#include "print.h"


int cogen_program_header(FILE *fp, char *filename){
    fprintf(fp, "\t.file\t\"%s\"\n\t.text\n", filename);
}

int cogen_program_trailer(FILE *fp, char *filename){
    fprintf(fp, "\t.ident\t\"GCC: (Ubuntu 4.8.2-19ubuntu1) 4.8.2\"\n\t.section\t.note.GNU-stack,\"\",@progbits\n");
}

int cogen_program(FILE *fp, program_t p, char *filename){
    cogen_program_header(fp, filename);
    fun_def_list_t fd_list = p->fun_defs;
    int n = fun_def_list_sz(fd_list);
    int i;
    int *p_label_sz = (int *)malloc(sizeof(int));
    *p_label_sz = 0;
    for(i = 0; i < n; i++){
        fun_def_t fd = fun_def_list_get(fd_list, i);
        cogen_fun_def(fp, fd, filename, i, p_label_sz);
    }
    cogen_program_trailer(fp, filename);
    return 0;
}

int cogen_fun_def_header(FILE *fp, char *name){
    fprintf(fp, "\t.globl\t%s\n\t.type\t%s, @function\n", name, name);
    return 0;
}

int cogen_fun_def_trailer(FILE *fp, char *name){
    fprintf(fp, "\t.size\t%s, .-%s\n", name, name);
    return 0;
}

int cogen_fun_def_prologue(FILE *fp, char *name, int index, int frame_sz){
    if(frame_sz == 0){
        fprintf(fp, "%s:\n.LFB%d:\n\t.cfi_startproc\n\tpushl\t%%ebp\n\t.cfi_def_cfa_offset 8\n\t.cfi_offset 5, -8\n\tmovl\t%%esp, %%ebp\n\t.cfi_def_cfa_register 5\n\tpushl\t%%ebx\n", name, index);
    }
    else{
        fprintf(fp, "%s:\n.LFB%d:\n\t.cfi_startproc\n\tpushl\t%%ebp\n\t.cfi_def_cfa_offset 8\n\t.cfi_offset 5, -8\n\tmovl\t%%esp, %%ebp\n\t.cfi_def_cfa_register 5\n\tpushl\t%%ebx\n\tsubl\t$%d, %%esp\n", name, index, frame_sz * 4);
    }

    return 0;
}

int cogen_fun_def_epilogue(FILE *fp, char *name, int index, int frame_sz, environment_t env){
    if(env->return_label != NULL){
        fprintf(fp, "%s:\n", env->return_label);
    }
    fprintf(fp, "\taddl\t$%d, %%esp\n", env->esp_sz * 4);
    pr_single_op(fp, "popl", "%ebx");
    pr_single_op(fp, "popl", "%ebp");
    fprintf(fp, "\t.cfi_restore 5\n\t.cfi_def_cfa 4, 4\n\tret\n\t.cfi_endproc\n.LFE%d:\n", index);
    return 0;
}

int cogen_alloc_storage_fun_def(fun_def_t fd){
    return ((list_t)(fd->params))->n;
}

int cogen_fun_def(FILE *fp, fun_def_t fd, char *filename, int index, int *p_label_sz){
    int frame_sz = cogen_alloc_storage_fun_def(fd);
    environment_t env = mk_environment(*p_label_sz);
    env->esp_sz += frame_sz;
    set_environment_params(env, fd->params);
    cogen_fun_def_header(fp, fd->f);
    cogen_fun_def_prologue(fp, fd->f, index, frame_sz);
    cogen_stmt(fp, fd->body, env);
    cogen_fun_def_epilogue(fp, fd->f, index, frame_sz, env);
    cogen_fun_def_trailer(fp, fd->f);
    *p_label_sz = env->label_sz;
}

int cogen_stmt(FILE *fp, stmt_t s, environment_t env){
    switch(s->kind){
    case stmt_kind_empty:		/* 空文 ; */
        return cogen_stmt_kind_empty(fp, s, env);
    case stmt_kind_continue:		/* continue ; */
        return cogen_stmt_kind_continue(fp, s, env);
    case stmt_kind_break:		/* break ; */
        return cogen_stmt_kind_break(fp, s, env);
    case stmt_kind_return:		/* return 式 ; */
        return cogen_stmt_kind_return(fp, s, env);
    case stmt_kind_expr:		/* 式文 ; (f(x, y); など) */
        return cogen_stmt_kind_expr(fp, s, env);
    case stmt_kind_compound:		/* 複合文 { ... } */
        return cogen_stmt_kind_compound(fp, s, env);
    case stmt_kind_if:			/* if文 */
        return cogen_stmt_kind_if(fp, s, env);
    case stmt_kind_while:		/* while文 */
        return cogen_stmt_kind_while(fp, s, env);
    }
}

int cogen_stmt_kind_empty(FILE *fp, stmt_t s, environment_t env){
}

int cogen_stmt_kind_continue(FILE *fp, stmt_t s, environment_t env){
    char *label = get_head_continue_label(env);
    pr_single_op(fp, "jmp", label);
}

int cogen_stmt_kind_break(FILE *fp, stmt_t s, environment_t env){
    char *label = get_break_label(env);
    pr_single_op(fp, "jmp", label);
}

int cogen_stmt_kind_return(FILE *fp, stmt_t s, environment_t env){
    char *ret = cogen_expr(fp, s->u.e, env);
    if(strcmp(ret, "%eax") != 0){
        pr_double_op(fp, "movl", ret, "%eax");
        pr_single_op(fp, "jmp", get_return_label(env));
    }
}

int cogen_stmt_kind_expr(FILE *fp, stmt_t s, environment_t env){
    cogen_expr(fp, s->u.e, env);
    free_all_reg(env);
}


int cogen_stmt_kind_compound(FILE *fp, stmt_t s, environment_t env){
    set_environment_variables(env, s->u.c.decls);
    int sz = ((list_t)s->u.c.decls)->n;
    if(sz != 0){
        fprintf(fp, "\tsubl\t$%d, %%esp\n", sz * 4);
        env->esp_sz += sz;
    }
    int i;
    for(i = 0; i < ((list_t)(s->u.c.body))->n; i++){
        cogen_stmt(fp, stmt_list_get(s->u.c.body, i), env);
    }
}

char *op_kind_to_jmp_str_for_if(op_kind_t op){
    switch(op){
    case op_kind_eq:
        return "jne";
    case op_kind_neq:
        return "je";
    case op_kind_ge:
        return "jl";
    case op_kind_gt:
        return "jle";
    case op_kind_le:
        return "jg";
    case op_kind_lt:
        return "jge";
    }
}

char *op_kind_to_jmp_str_for_while(op_kind_t op){
    switch(op){
    case op_kind_eq:
        return "je";
    case op_kind_neq:
        return "jne";
    case op_kind_ge:
        return "jge";
    case op_kind_gt:
        return "jg";
    case op_kind_le:
        return "jle";
    case op_kind_lt:
        return "jl";
    }
}


int is_relational(op_kind_t op){
    return (op == op_kind_eq || op == op_kind_neq || op == op_kind_ge || op == op_kind_gt || op == op_kind_le || op == op_kind_lt);
}

int cogen_stmt_kind_if(FILE *fp, stmt_t s, environment_t env){
    char *then_label = get_label(env);
    char *ex = cogen_expr(fp, s->u.i.e, env);
    //条件文が単なる数字 if(1)とか
    if(s->u.i.e->kind == expr_kind_int_literal){
        //条件文が0 -> elseがあればelse
        if(atoi(s->u.i.e->u.s) == 0){
            if(s->u.i.el != NULL){
                cogen_stmt(fp, s->u.i.el, env);
            }
        }
        //条件文が0以外 -> then
        else{
            cogen_stmt(fp, s->u.i.th, env);
        }
    }else{
        //条件文が比較文
        if(is_relational(s->u.i.e->u.a.o)){
            //比較文に応じたjmp命令
            pr_single_op(fp, op_kind_to_jmp_str_for_if(s->u.i.e->u.a.o), then_label);
        }
        else{
            char *reg = try_to_lock(env);
            pr_double_op(fp, "movl", ex, reg);
            pr_double_op(fp, "testl", reg, reg);
            pr_single_op(fp, "jne", then_label);
            try_to_free(env, reg);
        }
        cogen_stmt(fp, s->u.i.th, env);
        //else文がある場合
        if(s->u.i.el != NULL){
            char *else_label = get_label(env);
            pr_single_op(fp, "jmp", else_label);
            pr_label(fp, then_label);
            cogen_stmt(fp, s->u.i.el, env);
            pr_label(fp, else_label);
        }
        //else文がない場合
        else{
            pr_label(fp, then_label);
        }
    }
}

int cogen_stmt_kind_while(FILE *fp, stmt_t s, environment_t env){
    char *first_label = get_label(env);
    char *second_label = get_label(env);
    push_continue_label(env, first_label);
    pr_single_op(fp, "jmp", first_label);
    pr_label(fp, second_label);
    cogen_stmt(fp, s->u.w.body, env);
    pr_label(fp, first_label);
    char *ex = cogen_expr(fp, s->u.w.e, env);
    if(s->u.w.e->kind == expr_kind_int_literal){
        if(atoi(s->u.w.e->u.s) == 0){
            //何もしない
        }else{
            pr_single_op(fp, "jmp", second_label);
        }
    }else{
        if(is_relational(s->u.i.e->u.a.o)){
            pr_single_op(fp, op_kind_to_jmp_str_for_while(s->u.w.e->u.a.o), second_label);
            //fprintf(fp, "\t%s\t%s\n", op_kind_to_jmp_str_for_while(s->u.w.e->u.a.o), second_label);
        }
        else{
            char *reg = try_to_lock(env);
            pr_double_op(fp, "movl", ex, reg);
            pr_double_op(fp, "testl", reg, reg);
            pr_single_op(fp, "jne", second_label);
            try_to_free(env, reg);
        }
    }
    if(is_break_label_set(env)){
        pr_label(fp, env->break_label);
        free_break_label(env);
    }
    pop_continue_label(env);
}

char *cogen_expr(FILE *fp, expr_t e, environment_t env){

    switch(e->kind){
    case expr_kind_int_literal:
        return cogen_expr_kind_int_literal(fp, e, env);
    case expr_kind_id:
        return cogen_expr_kind_id(fp, e, env);
    case expr_kind_paren:
        return cogen_expr_kind_paren(fp, e, env);
    case expr_kind_app:
        return cogen_expr_kind_app(fp, e, env);
    }
}

char *cogen_expr_kind_int_literal(FILE *fp, expr_t e, environment_t env){
    char *str = (char *)malloc(sizeof(char) * 11);
    sprintf(str, "$%d", atoi(e->u.s));
    return str;
}

char *cogen_expr_kind_id(FILE *fp, expr_t e, environment_t env){
    return get_environment_variable_addr(env, e->u.s);
}

char *cogen_expr_kind_paren(FILE *fp, expr_t e, environment_t env){
    return cogen_expr(fp, e->u.p, env);
}

char *cogen_expr_additive_recursive(FILE *fp, expr_t e, environment_t env, char *reg, int op){
    expr_t e0 = expr_list_get(e->u.a.args, 0);
    expr_t e1 = expr_list_get(e->u.a.args, 1);
    char *r0 = cogen_expr(fp, e0, env);
    if(op == -1){
        pr_double_op(fp, "movl", r0, reg);
    }
    else if(op == 0){
        pr_double_op(fp, "addl", r0, reg);
    }
    else if(op == 1){
        pr_double_op(fp, "subl", r0, reg);
    }
    try_to_free(env, r0);
    if(e1->kind == expr_kind_app && (e1->u.a.o == op_kind_bin_plus || e1->u.a.o == op_kind_bin_minus)){
        if(e->u.a.o == op_kind_bin_plus){
            return cogen_expr_additive_recursive(fp, e1, env, reg, 0);
        }
        else if(e->u.a.o == op_kind_bin_minus){
            return cogen_expr_additive_recursive(fp, e1, env, reg, 1);
        }
    }
    char *r1 = cogen_expr(fp, e1, env);
    if(e->u.a.o == op_kind_bin_plus){
        pr_double_op(fp, "addl", r1, reg);
    }
    else if(e->u.a.o == op_kind_bin_minus){
        pr_double_op(fp, "subl", r1, reg);
    }
    try_to_free(env, r1);
    return reg;
}

char *cogen_expr_multiplicative_recursive(FILE *fp, expr_t e, environment_t env, char *reg, int op){
    expr_t e0 = expr_list_get(e->u.a.args, 0);
    expr_t e1 = expr_list_get(e->u.a.args, 1);
    char *r0 = cogen_expr(fp, e0, env);
    if(op == -1){
        pr_double_op(fp, "movl", r0, reg);
    }
    else if(op == 0){
        pr_double_op(fp, "imull", r0, reg);
    }
    else if(op == 1){
        pr_double_op(fp, "movl", reg, "%eax");
        if(r0[0] == '$'){
            char *reg_tmp = try_to_lock(env);
            pr_double_op(fp, "movl", r0, reg_tmp);
            fprintf(fp, "\tcltd\n");
            pr_single_op(fp, "idivl", reg_tmp);
            pr_double_op(fp, "movl", "%eax", reg);
            try_to_free(env, reg_tmp);
        }
        else{
            fprintf(fp, "\tcltd\n");
            pr_single_op(fp, "idivl", r0);
            pr_double_op(fp, "movl", "%eax", reg);
        }
    }
    else if(op == 2){
        pr_double_op(fp, "movl", reg, "%eax");
        if(r0[0] == '$'){
            char *reg_tmp = try_to_lock(env);
            pr_double_op(fp, "movl", r0, reg_tmp);
            fprintf(fp, "\tcltd\n");
            pr_single_op(fp, "idivl", reg_tmp);
            pr_double_op(fp, "movl", "%edx", reg);
            try_to_free(env, reg_tmp);
        }
        else{
            fprintf(fp, "\tcltd\n");
            pr_single_op(fp, "idivl", r0);
            pr_double_op(fp, "movl", "%edx", reg);
        }
    }
    try_to_free(env, r0);
    if(e1->kind == expr_kind_app && (e1->u.a.o == op_kind_mult || e1->u.a.o == op_kind_div || e1->u.a.o == op_kind_rem)){
        if(e->u.a.o == op_kind_mult){
            return cogen_expr_multiplicative_recursive(fp, e1, env, reg, 0);
        }
        else if(e->u.a.o == op_kind_div){
            return cogen_expr_multiplicative_recursive(fp, e1, env, reg, 1);
        }
        else if(e->u.a.o == op_kind_rem){
            return cogen_expr_multiplicative_recursive(fp, e1, env, reg, 2);
        }
    }
    char *r1 = cogen_expr(fp, e1, env);
    if(e->u.a.o == op_kind_mult){
        pr_double_op(fp, "imull", r1, reg);
    }
    else if(e->u.a.o == op_kind_div){
        pr_double_op(fp, "movl", reg, "%eax");
        if(r1[0] == '$'){
            char *reg_tmp = try_to_lock(env);
            pr_double_op(fp, "movl", r1, reg_tmp);
            fprintf(fp, "\tcltd\n");
            pr_single_op(fp, "idivl", reg_tmp);
            pr_double_op(fp, "movl", "%eax", reg);
            try_to_free(env, reg_tmp);
        }
        else{
            fprintf(fp, "\tcltd\n");
            pr_single_op(fp, "idivl", r1);
            pr_double_op(fp, "movl", "%eax", reg);
        }
    }
    else if(e->u.a.o == op_kind_rem){
        pr_double_op(fp, "movl", reg, "%eax");
        if(r1[0] == '$'){
            char *reg_tmp = try_to_lock(env);
            pr_double_op(fp, "movl", r1, reg_tmp);
            fprintf(fp, "\tcltd\n");
            pr_single_op(fp, "idivl", reg_tmp);
            pr_double_op(fp, "movl", "%edx", reg);
            try_to_free(env, reg_tmp);
        }
        else{
            fprintf(fp, "\tcltd\n");
            pr_single_op(fp, "idivl", r1);
            pr_double_op(fp, "movl", "%edx", reg);
        }
    }
    try_to_free(env, r1);
    return reg;
}

char *cogen_expr_kind_app(FILE *fp, expr_t e, environment_t env){
    expr_t e0, e1;

    switch (e->u.a.o){
    /* 関数呼び出し */
    case op_kind_fun:{
        char *func = e->u.a.f;
        int arg_sz = expr_list_sz(e->u.a.args);
        int i;
        for(i = arg_sz - 1; i >= 0; i--){
            char *str = cogen_expr(fp, expr_list_get(e->u.a.args, i), env);
            if(i == 0){
                if(isdigit(str[0]) || str[0] == '-'){
                    char *reg = try_to_lock(env);
                    pr_double_op(fp, "movl", str, reg);
                    pr_double_op(fp, "movl", reg, "(%esp)");
                    try_to_free(env, reg);
                }
                else{
                    pr_double_op(fp, "movl", str, "(%esp)");
                }
            }
            else{
                if(isdigit(str[0]) || str[0] == '-'){
                    char *reg = try_to_lock(env);
                    pr_double_op(fp, "movl", str, reg);
                    fprintf(fp, "\tmovl\t%s, %d(%%esp)\n", reg, 4 * i);
                    try_to_free(env, reg);
                }
                else{
                    fprintf(fp, "\tmovl\t%s, %d(%%esp)\n", str, 4 * i);
                }
            }
            try_to_free(env, str);
        }
        pr_single_op(fp, "call", func);
        if(env->reg_flags[reg_to_index("%ebx")]){
            char *ret_reg = try_to_lock(env);
            pr_double_op(fp, "movl", "%eax", ret_reg);
            return ret_reg;
        }
        else{
            lock_reg(env, "%ebx");
            pr_double_op(fp, "movl", "%eax", "%ebx");
            return "%ebx";
        }
    }

    /* a = b */
    case op_kind_assign:{
        e0 = expr_list_get(e->u.a.args, 0);
        e1 = expr_list_get(e->u.a.args, 1);
        //左辺値じゃなかったらエラーだよなあ
        char *left = cogen_expr(fp, e0, env);
        char *right = cogen_expr(fp, e1, env);
        if((isdigit(left[0]) || left[0] == '-') && (isdigit(right[0]) || right[0] == '-')){
            char *reg = try_to_lock(env);
            pr_double_op(fp, "movl", right, reg);
            pr_double_op(fp, "movl", reg, left);
            try_to_free(env, reg);
        }else{
            pr_double_op(fp, "movl", right, left);
        }
        try_to_free(env, right);
        return left;
    }

    /* a == b */
    case op_kind_eq:
    /* a != b */
    case op_kind_neq:
    /* a < b */
    case op_kind_lt:
    /* a > b */
    case op_kind_gt:
    /* a <= b */
    case op_kind_le:
    /* a >= b */
    case op_kind_ge:{
        e0 = expr_list_get(e->u.a.args, 0);
        e1 = expr_list_get(e->u.a.args, 1);
        char *left = cogen_expr(fp, e0, env);
        char *right = cogen_expr(fp, e1, env);
        if((isdigit(left[0]) || left[0] == '-') && (isdigit(right[0]) || right[0] == '-')){
            char *reg = try_to_lock(env);
            pr_double_op(fp, "movl", left, reg);
            pr_double_op(fp, "cmpl", right, reg);
            try_to_free(env, reg);
        }
        else if(left[0] == '$'){
            char *reg = try_to_lock(env);
            pr_double_op(fp, "movl", left, reg);
            pr_double_op(fp, "cmpl", right, reg);
        }
        else{
            pr_double_op(fp, "cmpl", right, left);
        }
        try_to_free(env, right);
        try_to_free(env, left);
        return NULL;
    }

    /* a + b */
    case op_kind_bin_plus:{
        char *reg = cogen_expr_additive_recursive(fp, e, env, try_to_lock(env),-1);
        return reg;
    }
    /* a - b */
    case op_kind_bin_minus:{
        char *reg = cogen_expr_additive_recursive(fp, e, env, try_to_lock(env), -1);
        return reg;
    }
    /* a * b */
    case op_kind_mult:{
        char *reg = cogen_expr_multiplicative_recursive(fp, e, env, try_to_lock(env), -1);
        return reg;
    }
    /* a / b */
    case op_kind_div:{
        char *reg = cogen_expr_multiplicative_recursive(fp, e, env, try_to_lock(env), -1);
        return reg;
    }

    /* a % b */
    case op_kind_rem:{
        char *reg = cogen_expr_multiplicative_recursive(fp, e, env, try_to_lock(env), -1);
        return reg;
    }
    /* +a 単項+ */
    case op_kind_un_plus:{
        e0 = expr_list_get(e->u.a.args, 0);
        return cogen_expr(fp, e0, env);
    }
    /* -a 単項- */
    case op_kind_un_minus:{
        e0 = expr_list_get(e->u.a.args, 0);
        char *str = cogen_expr(fp, e0, env);
        if(is_reg(str)){
            pr_single_op(fp, "negl", str);
            return str;
        }
        else{
            char *reg = try_to_lock(env);
            pr_double_op(fp, "movl", str, reg);
            pr_single_op(fp, "negl", reg);
            return reg;
        }
    }
	/* !a */
    case op_kind_logneg:{
        e0 = expr_list_get(e->u.a.args, 0);
        char *str = cogen_expr(fp, e0, env);
        pr_double_op(fp, "cmpl", "$0", str);
        try_to_free(env, str);
        pr_single_op(fp, "sete", "%al");
        char *reg = try_to_lock(env);
        pr_double_op(fp, "movzbl", "%al", reg);
        return reg;
    }
    }
}

