#ifndef _UTILS_H_
#define _UTILS_H_

#include "global.h"

char *str_cpy(char *dest);
void sighandler();
char *get_basename(char *buf, int buf_size);
char *complete_addr(char *cmdname, char *full_addr);
void raise_error_free(char *err, char *input_cpy, char **commands);
void i_o_counter(char *command, int *results);

#endif