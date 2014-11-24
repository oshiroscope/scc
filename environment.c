#include <stdio.h>
#include <string.h>
#include "list.h"
#include "syntree.h"
#include "environment.h"
#include "util.h"

environment_t mk_environment(int label_sz){
    environment_t env = (environment_t)safe_malloc(sizeof(struct environment));
    env->allow_sz = 10;
    env->vars = (var_addr_t *)malloc(sizeof(struct var_addr *) * env->allow_sz);//(char **)malloc(sizeof(char *) * env->allow_sz);
    int i;
    for(i = 0; i < env->allow_sz; i++){
        env->vars[i] = (var_addr_t)malloc(sizeof(struct var_addr));
    }
    env->var_sz = 0;
    env->arg_sz = 0;
    for(i = 0; i < 3; i++){
        env->reg_flags[i] = 0;
    }
    env->label_sz = label_sz;
    env->break_label = NULL;
    env->continue_labels = (char **)malloc(sizeof(char *)*10);
    env->continue_label_sz = 0;
    env->return_label = NULL;
    return env;
}

void error(){
    fprintf(stderr, "error\n");
    exit(1);
}

void set_environment_params(environment_t env, var_decl_list_t params){
    int i;

    while(env->var_sz + env->arg_sz + ((list_t)params)->sz > env->allow_sz){
        int old_sz = env->allow_sz;
        env->allow_sz *=2;
        env->vars = (var_addr_t)realloc(env->vars, sizeof(struct var_addr) * env->allow_sz);
        for(i = old_sz; i < env->allow_sz; i++){
            env->vars[i] = (var_addr_t)malloc(sizeof(struct var_addr));
        }
    }

    for(i = 0; i < ((list_t)params)->n; i++){
        env->arg_sz++;
        env->vars[i]->name = strdup(((var_decl_t)(((list_t)params)->a[i]))->v);
        //env->vars[i]->name = ((var_decl_t)(((list_t)params)->a[i]))->v;
        char addr[10]; //8(%ebp)みたいな
        sprintf(addr, "%d(%%ebp)", env->arg_sz*4 + 4);
        env->vars[i]->addr = strdup(addr);
    }
}

void set_environment_variables(environment_t env, var_decl_list_t decls){
    int i;
    int cur_sz = env->var_sz + env->arg_sz;
    while(cur_sz + ((list_t)decls)->n > env->allow_sz){
        int old_sz = env->allow_sz;
        env->allow_sz *=2;
        env->vars = (var_addr_t)realloc(env->vars, sizeof(struct var_addr) * env->allow_sz);
        for(i = old_sz; i < env->allow_sz; i++){
            env->vars[i] = (var_addr_t)malloc(sizeof(struct var_addr));
        }
    }

    for(i = 0; i < ((list_t)decls)->n; i++){
        env->var_sz++;
         env->vars[i + cur_sz]->name = strdup(((var_decl_t)(((list_t)decls)->a[i]))->v);
        char addr[10]; //-4(%ebp)みたいな
        sprintf(addr, "-%d(%%ebp)", env->var_sz*4);
        env->vars[i + cur_sz]->addr = strdup(addr);
    }
}

char *get_environment_variable_addr(environment_t env, char *name){
    int i;
    for(i = 0; i < env->var_sz + env->arg_sz; i++){
        if(strcmp(env->vars[i]->name, name) == 0){
            return env->vars[i]->addr;
        }
    }
    error();
}

char *index_to_reg(int id){
    switch(id){
    case 0:
        return "%ebx";
    case 1:
        return "%ecx";
    case 2:
        return "%edx";
    default:
        error();
    }
}

int reg_to_index(char *reg){
    if(strcmp(reg, "%ebx") == 0) return 0;
    else if(strcmp(reg, "%ecx") == 0) return 1;
    else if(strcmp(reg, "%edx") == 0) return 2;
    else error();
}

char *lookup_flag(environment_t env){
    int i;
    for(i = 0; i < 3; i++){
        if(env->reg_flags[i] == 0) return index_to_reg(i);
    }
    error();
}

void lock_reg(environment_t env, char *reg){
    env->reg_flags[reg_to_index(reg)] = 1;
}

void free_reg(environment_t env, char *reg){
    env->reg_flags[reg_to_index(reg)] = 0;
}

void free_all_reg(environment_t env){
    int i;
    for(i = 0; i < 3; i++){
        env->reg_flags[i] = 0;
    }
}

int is_reg(char *str){
    return strcmp(str, "%ebx") == 0 || strcmp(str, "%ecx") == 0 || strcmp(str, "%edx") == 0;
}

char *try_to_lock(environment_t env){
    char *reg = lookup_flag(env);
    lock_reg(env, reg);
    return reg;
}

void try_to_free(environment_t env, char *str){
    if(is_reg(str)){
        free_reg(env, str);
    }
}

char *get_label(environment_t env){
    env->label_sz++;
    char *label = (char *)malloc(sizeof(char) * 6);
    sprintf(label, ".L%d", env->label_sz + 1);
    return label;
}

char *get_break_label(environment_t env){
    if(env->break_label == NULL){
        env->break_label = get_label(env);
    }
    return env->break_label;
}

void free_break_label(environment_t env){
    if(env->break_label == NULL){
        error();
    }
    else{
        env->break_label = NULL;
    }
}

int is_break_label_set(environment_t env){
    return env->break_label != NULL;
}

void push_continue_label(environment_t env, char *label){
    env->continue_labels[env->continue_label_sz] = label;
    env->continue_label_sz++;
}

char *get_head_continue_label(environment_t env){
    return env->continue_labels[env->continue_label_sz - 1];
}

char *pop_continue_label(environment_t env){
    env->continue_label_sz--;
    env->continue_labels[env->continue_label_sz] = NULL;
}

char *get_return_label(environment_t env){
    if(env->return_label == NULL){
        env->return_label = get_label(env);
    }
    return env->return_label;
}
