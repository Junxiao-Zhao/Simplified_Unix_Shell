#ifndef _EXECUTE_H_
#define _EXECUTE_H_

#include "global.h"

void subprocess(char **argv);
void child_process(char *command, int status, int *pipe_in, int *pipe_out);
void i_o_processer(char *command, int *pipe_in, int *pipe_out);
void pipe_processer(char **commands, int *i_o_status);
void built_ins(char *cmdname, char *rest_arg);

#endif