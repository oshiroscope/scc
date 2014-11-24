#pragma once
#include "syntree.h"

typedef struct var_addr * var_addr_t;

struct var_addr{
    char *name;
    char *addr;
};

typedef struct environment * environment_t;

struct environment{
    var_addr_t *vars;
    int var_sz;
    int allow_sz;
    int arg_sz;
    int reg_flags[3];
    int label_sz;
    char *break_label;
    char **continue_labels;
    int continue_label_sz;
    char *return_label;
    int esp_sz;
};

environment_t mk_environment(int label_sz);
void set_environment_params(environment_t env, var_decl_list_t params);
void set_environment_variables(environment_t env, var_decl_list_t decls);

char *get_environment_variable_addr(environment_t env, char *name);
void inherit_environment(environment_t child, environment_t parent);

char *index_to_reg(int id);

int reg_to_index(char *reg);

char *lookup_flag(environment_t env);

void lock_reg(environment_t env, char *reg);

void free_reg(environment_t env, char *reg);

void free_all_reg(environment_t env);

char *try_to_lock(environment_t env);
void try_to_free(environment_t env, char *str);

int is_reg(char *str);

char *get_label(environment_t env);

char *get_break_label(environment_t env);
void free_break_label(environment_t env);
int is_break_label_set(environment_t env);

void push_continue_label(environment_t env, char *label);
char *get_head_continue_label(environment_t env);
char *pop_continue_label(environment_t env);

char *get_return_label(environment_t env);

