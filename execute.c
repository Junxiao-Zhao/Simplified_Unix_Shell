#include "global.h"

int errno;

typedef struct suspended
{
    char *command;
    pid_t pid;
    struct suspended *next;
} LinkList;

LinkList *head;
int cur_pos = 0;

// add suspended commands to list
void add_suspend(char *command, pid_t child_pid)
{
    LinkList *node = (LinkList *)malloc(sizeof(LinkList));
    node->command = command;
    node->pid = child_pid;
    node->next = NULL;

    if (!cur_pos)
        head = node;
    else
        head->next = node;

    cur_pos++;
}

// execute external programs without Pipe (not built-in)
void child_process(char *ex_command, int status, int *pipe_in, int *pipe_out)
{
    char **argv = (char **)malloc(sizeof(char *) * 501);
    char *rest_args = NULL;

    char *command = str_cpy(ex_command);

    int i = 0;
    while (NULL != (command = strtok_r(command, " ", &rest_args)))
    {
        // Complete the basename into absolute path
        if (i == 0)
        {
            // alloc enough space for complete the absolute address
            char *usrbin = (char *)malloc(sizeof(char) * strlen(command) + 10);
            strcpy(usrbin, "/usr/bin/");
            usrbin[9] = '\0';
            command = complete_addr(command, usrbin);
        }

        argv[i++] = command;
        command = NULL;
    }
    argv[i] = NULL;

    errno = 0; // reset errno to 0

    // Child process
    if (status == 2) // no I/O
    {
        pid_t cur_pid = fork();

        // failed
        if (cur_pid == -1)
            fprintf(stderr, "Failed to create child processes\n");

        // child
        else if (cur_pid == 0)
        {
            // Restore SIGINT, SIGQUIT, SIGTSTP in the child process
            signal(SIGINT, SIG_DFL);
            signal(SIGQUIT, SIG_DFL);
            signal(SIGTSTP, SIG_DFL);

            // input redirect in Pipe
            if (pipe_in != NULL)
            {
                close(0); // close stdin
                close(pipe_in[1]);
                dup2(pipe_in[0], 0);
                close(pipe_in[0]);
            }
            // output redirect in Pipe
            if (pipe_out != NULL)
            {
                close(1);           // close stdout
                close(pipe_out[0]); // close read
                dup2(pipe_out[1], 1);
                close(pipe_out[1]); // close write
            }

            execv(argv[0], argv);

            if (errno != 0)
                fprintf(stderr, "Error: invalid program\n");

            exit(errno);
        }
        // shell
        else
        {
            if (pipe_in != NULL)
                close(pipe_in[0]); // close read

            if (pipe_out != NULL)
                close(pipe_out[1]); // close write

            int child_status;
            waitpid(cur_pid, &child_status, WUNTRACED);

            // if child process is suspended
            if (WIFSTOPPED(child_status))
                add_suspend(ex_command, cur_pid);
        }

        free(argv);
    }
    else // with I/O
    {
        execv(argv[0], argv);

        if (errno != 0)
            fprintf(stderr, "Error: invalid program\n");

        exit(errno);
    }
}

// execute the command with I/O
void i_o_processer(char *command, int *pipe_in, int *pipe_out)
{
    errno = 0; // reset errno to 0

    pid_t cur_pid = fork();

    // failed
    if (cur_pid == -1)
        fprintf(stderr, "Failed to create child processes\n");

    // child
    else if (cur_pid == 0)
    {
        // Restore SIGINT, SIGQUIT, SIGTSTP in the child process
        signal(SIGINT, SIG_DFL);
        signal(SIGQUIT, SIG_DFL);
        signal(SIGTSTP, SIG_DFL);

        // input redirect in Pipe
        if (pipe_in != NULL)
        {
            close(0); // close stdin
            close(pipe_in[1]);
            dup2(pipe_in[0], 0);
            close(pipe_in[0]);
        }
        // output redirect in Pipe
        if (pipe_out != NULL)
        {
            close(1); // close stdout
            close(pipe_out[0]);
            dup2(pipe_out[1], 1);
            close(pipe_out[1]);
        }

        // copy the command
        char *cmd_cpy = str_cpy(command);

        // find the place of redicrections
        char *in = strstr(cmd_cpy, "<");
        char *out = strstr(cmd_cpy, " > ");
        char *out2 = strstr(cmd_cpy, ">>");

        // only output
        if (in == NULL && (out != NULL || out2 != NULL))
        {
            char *filename = NULL;
            char *pre_i_o = strtok_r(cmd_cpy, ">", &filename); // the command before redirect

            // open the file
            int fd;
            if (out2 != NULL) // append
            {
                strtok_r(filename, " ", &filename);
                filename = strtok(filename, " ");
                fd = open(filename, O_WRONLY | O_CREAT | O_APPEND, 0644);
            }
            else // overwrite
            {
                filename = strtok(filename, " ");
                fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            }

            // failed to open the file
            if (fd == -1)
            {
                fprintf(stderr, "Error opening file: %s\n", strerror(errno));
                free(cmd_cpy);
                exit(errno);
            }

            close(1); // close stdout
            // redirect stdout
            dup2(fd, 1);
            close(fd);

            child_process(pre_i_o, 1, pipe_in, pipe_out);
            close(1);
        }

        // only input
        else if (in != NULL && out == NULL && out2 == NULL)
        {
            char *filename = NULL;
            char *pre_i_o = strtok_r(cmd_cpy, "<", &filename); // the command before redirect

            // open the file
            filename = strtok(filename, " ");
            int fd = open(filename, O_RDONLY, 0644);

            // file not exist
            if (fd == -1)
            {
                fprintf(stderr, "Error: invalid file\n");
                free(cmd_cpy);
                exit(errno);
            }

            close(0); // close stdin
            // redirect stdin
            dup2(fd, 0);
            close(fd);

            child_process(pre_i_o, 1, pipe_in, pipe_out);
            close(0);
        }

        // both input and output
        else
        {
            char *in_file;
            char *out_file;
            char *pre_i_o;
            char *post_i_o;

            int fd_out;
            // input before output
            if (in < out || in < out2)
            {
                pre_i_o = strtok_r(cmd_cpy, "<", &post_i_o);  // the command before input redirect
                in_file = strtok_r(post_i_o, ">", &out_file); // extract input file
                in_file = strtok(in_file, " ");

                // append
                if (out2 != NULL)
                {
                    strtok_r(out_file, " ", &out_file);
                    out_file = strtok(out_file, " ");
                    fd_out = open(out_file, O_RDWR | O_CREAT | O_APPEND, 0644);
                }
                // overwrite
                else
                {
                    out_file = strtok(out_file, " ");
                    fd_out = open(out_file, O_RDWR | O_CREAT | O_TRUNC, 0644);
                }
            }
            // output before input
            else
            {
                pre_i_o = strtok_r(cmd_cpy, "<", &in_file); // the command before input redirect
                in_file = strtok(in_file, " ");
                pre_i_o = strtok_r(pre_i_o, ">", &post_i_o); // the command before output redirect

                // append
                if (out2 != NULL)
                {
                    strtok_r(post_i_o, " ", &post_i_o);
                    out_file = strtok(post_i_o, " ");
                    fd_out = open(out_file, O_RDWR | O_CREAT | O_APPEND, 0644);
                }
                // overwrite
                else
                {
                    out_file = strtok(post_i_o, " ");
                    fd_out = open(out_file, O_RDWR | O_CREAT | O_TRUNC, 0644);
                }
            }

            int fd_in = open(in_file, O_RDONLY, 0644);
            // file not exist
            if (fd_in == -1)
            {
                fprintf(stderr, "Error: invalid file\n");
                free(cmd_cpy);
                exit(errno);
            }

            close(1); // close stdout
            // redirect stdout
            dup2(fd_out, 1);
            close(fd_out);

            close(0); // close stdin
            // redirect stdin
            dup2(fd_in, 0);
            close(fd_in);

            child_process(pre_i_o, 1, pipe_in, pipe_out);
            close(0);
            close(1);
        }
    }
    // shell
    else
    {
        if (pipe_in != NULL)
            close(pipe_in[0]); // close read

        if (pipe_out != NULL)
            close(pipe_out[1]); // close write

        int child_status;
        waitpid(cur_pid, &child_status, WUNTRACED);

        // if child process is suspended
        if (WIFSTOPPED(child_status))
            add_suspend(command, cur_pid);
    }
}

// execute the command with Pipe
void pipe_processer(char **commands, int *i_o_status)
{
    errno = 0; // reset errno to 0

    int pipe0[2];
    int pipe1[2];
    int *pipe_in = pipe0;
    int *pipe_out = pipe1;

    int i = 0;
    while (commands[i] != NULL)
    {
        // first program only redirect stdout
        if (i == 0)
            pipe_in = NULL;

        // last program only redirect stdin
        if (commands[i + 1] == NULL)
            pipe_out = NULL;

        // initialize stdout
        else
        {
            if (pipe(pipe_out) == -1)
            {
                fprintf(stderr, "create pipe failed\n");
                exit(errno);
            }
        }

        // child process
        if (i_o_status[i] == 1)
            i_o_processer(commands[i], pipe_in, pipe_out);

        else
            child_process(commands[i], 2, pipe_in, pipe_out);

        // switch pipe0 and pipe1 as pipe_in and pipe_out
        if (i % 2 == 0)
        {
            pipe_out = pipe0;
            pipe_in = pipe1;
        }
        else
        {
            pipe_in = pipe0;
            pipe_out = pipe1;
        }

        i++;
    }
}

// built-in commands
void built_ins(char *cmdname, char *rest_arg)
{
    errno = 0; // reset errno to 0

    // find the number of arguments
    char *args = NULL;
    rest_arg = strtok_r(rest_arg, " ", &args);

    if (strcmp(cmdname, "cd") == 0)
    {
        //  0 or 2+ args
        if (rest_arg == NULL || strstr(args, " ") != NULL)
            fprintf(stderr, "Error: invalid command\n");
        else
        {
            chdir(rest_arg);
            if (errno != 0)
                fprintf(stderr, "Error: invalid directory\n");
        }
    }

    else if (strcmp(cmdname, "exit") == 0)
    {
        if (rest_arg != NULL)
            fprintf(stderr, "Error: invalid command\n");
        else if (head != NULL)
            fprintf(stderr, "Error: there are suspended jobs\n");
        else
            exit(0);
    }

    else if (strcmp(cmdname, "fg") == 0)
    {
        //  0 or 2+ args
        if (rest_arg == NULL || strstr(args, " ") != NULL)
            fprintf(stderr, "Error: invalid command\n");
        else
        {
            int target = atoi(rest_arg); // the target process to resume

            int i = 1;
            LinkList *pre_node = NULL;
            LinkList *cur_node = head;
            while (cur_node != NULL)
            {
                if (i++ == target)
                {
                    if (kill(cur_node->pid, SIGCONT) == -1)
                        fprintf(stderr, "Failed to resume processes\n");
                    else
                    {
                        wait(NULL);
                        if (pre_node == NULL)
                            head = cur_node->next;
                        else
                            pre_node->next = cur_node->next;
                    }
                    return;
                }

                pre_node = cur_node;
                cur_node = cur_node->next;
            }
            fprintf(stderr, "Error: invalid job\n");
        }
    }

    else if (strcmp(cmdname, "jobs") == 0)
    {
        if (rest_arg != NULL)
            fprintf(stderr, "Error: invalid command\n");
        else
        {
            int i = 1;
            LinkList *cur_node = head;
            while (cur_node != NULL)
            {
                printf("[%d]%s\n", i++, cur_node->command);
                cur_node = cur_node->next;
            }
        }
    }
}