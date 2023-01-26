#include "global.h"

// Copy strings
char *str_cpy(char *dest)
{
    char *src = (char *)malloc(sizeof(char) * (strlen(dest) + 1));
    strcpy(src, dest);
    src[strlen(dest)] = '\0';

    return src;
}

// Get current working directory
char *get_basename(char *buf, int buf_size)
{
    getcwd(buf, buf_size);
    return basename(buf);
}

// Change basename into absolute path
char *complete_addr(char *cmdname, char *full_addr)
{
    // absolute or relative path
    if (cmdname[0] == '/' || strstr(cmdname, "/") != NULL)
    {
        return cmdname;
    }
    // only base name
    else
    {
        strcat(full_addr, cmdname);
        return full_addr;
    }
}

// TODO: change free part into va_list?
// raise error and free memory
void raise_error_free(char *err, char *input_cpy, char **commands)
{
    fprintf(stderr, err);
    free(input_cpy);
    free(commands);
}

// Count the number of input and output redirection
void i_o_counter(char *command, int *results)
{
    int i = 0;
    while (NULL != (command = strtok(command, " ")))
    {
        if (strcmp(command, "<") == 0)
        {
            results[0]++;
            results[4]++;
            i = 1;
        }
        else if (strcmp(command, ">") == 0)
        {
            results[1]++;
            results[4]++;
            i = 1;
        }
        else if (strcmp(command, ">>") == 0)
        {
            results[2]++;
            results[4]++;
            i = 1;
        }
        else if (strcmp(command, "<<") == 0)
        {
            results[3]++;
            results[4]++;
        }
        else if (i)
        {
            results[4]--;
        }

        command = NULL;
    }
}
