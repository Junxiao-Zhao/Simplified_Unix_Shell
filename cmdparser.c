#include "global.h"

// Check I/O
int i_o_checker(char *command, int pos)
{
    // Store the count {0: <, 1: >, 2: >>, 3: <<, 4: files}
    int *results = (int *)calloc(5, sizeof(int));
    // Copy the command
    char *cmd_cpy = str_cpy(command);

    // count the number of each redirection
    i_o_counter(cmd_cpy, results);

    // << is not valid
    if (results[3])
        return 0;

    // at most one input redirection and one output redirection
    else if (results[0] > 1 || (results[1] + results[2]) > 1)
        return 0;

    // the first program may redirect its input
    else if (pos == 0 && (results[1] + results[2]))
        return 0;

    // the last program may redirect its output
    else if (pos == -1 && results[0])
        return 0;

    // contain no I/O
    else if (!results[0] && !results[1] && !results[2])
        return 2;

    // check the number of files
    else
    {
        // contain I/O properly
        if (!results[4])
            return 1;
        else
            return 0;
    }
}

// Command Parser
void command_parser(char *input)
{
    // Empty input
    if (strcmp(input, "\n") == 0)
        return;

    // Copy the input
    char *input_cpy = (char *)malloc(sizeof(char) * (strlen(input) + 2));
    // Add blank space before and after the command for avoiding starting or ending with '|'
    input_cpy[0] = ' ';
    strncpy(input_cpy + 1, input, strlen(input) - 1);
    input_cpy[strlen(input)] = ' ';
    input_cpy[strlen(input) + 1] = '\0';

    int status; // I/O status

    // Check pipe
    if (strstr(input_cpy, "|") != NULL)
    {
        // alloc space for each command
        char **commands = (char **)malloc(sizeof(char *) * 251);
        // alloc space to store whether each command contains I/O
        int *i_o_status = (int *)malloc(sizeof(int) * 251);

        // Separate commands connected by pipe
        char *each_command = input_cpy;
        char *rest_cmds = NULL;

        int i = 0;
        int pos;
        while (NULL != (each_command = strtok_r(each_command, "|", &rest_cmds)))
        {
            // get the cmdname of each command
            char *each_c = str_cpy(each_command);
            char *cmdname = strtok(each_c, " ");

            // invalid command if it's built-in or blank space
            if (cmdname == NULL || !strcmp(cmdname, "cd") || !strcmp(cmdname, "exit") || !strcmp(cmdname, "fg") || !strcmp(cmdname, "jobs"))
            {
                raise_error_free("Error: invalid command\n", input_cpy, commands);
                return;
            }
            else
            {
                // pos is the indicator for first / last command
                if (i == 0)
                    pos = 0;
                else if (!strlen(rest_cmds))
                    pos = -1;
                else
                    pos = 2;

                status = i_o_checker(each_command, pos);
                if (status)
                {
                    commands[i] = each_command;
                    i_o_status[i++] = status;
                }
                else
                {
                    raise_error_free("Error: invalid command\n", input_cpy, commands);
                    return;
                }
            }
            each_command = NULL;
        }
        commands[i] = NULL;
        pipe_processer(commands, i_o_status);
    }
    // no Pipe
    else
    {
        // get the cmdname of the command
        char *each_c = str_cpy(input_cpy);
        char *rest_arg = NULL;
        char *cmdname = strtok_r(each_c, " ", &rest_arg);

        // built-in or space
        if (cmdname == NULL || !strcmp(cmdname, "cd") || !strcmp(cmdname, "exit") || !strcmp(cmdname, "fg") || !strcmp(cmdname, "jobs"))
        {
            // invalid command if contain I/O
            if (i_o_checker(each_c, 1) == 1)
                fprintf(stderr, "Error: invalid command\n");

            else
                built_ins(cmdname, rest_arg);
        }
        // other commands
        else
        {
            status = i_o_checker(input_cpy, 1);
            // contain I/O
            if (status == 1)
            {
                i_o_processer(input_cpy, NULL, NULL);
            }
            // not contain I/O
            else if (status == 2)
                child_process(input_cpy, status, NULL, NULL);
            // invalid command
            else
                fprintf(stderr, "Error: invalid command\n");
        }
    }
}