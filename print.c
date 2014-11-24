#include <stdio.h>
#include "print.h"

void pr_single_op(FILE *fp, char *operation, char *operand){
    fprintf(fp, "\t%s\t%s\n", operation, operand);
}

void pr_double_op(FILE *fp, char *operation, char *first, char *second){
    fprintf(fp, "\t%s\t%s, %s\n", operation, first, second);
}

void pr_label(FILE *fp, char *label){
    fprintf(fp, "%s:\n", label);
}
