#include <stdlib.h>
#include <assert.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "tests/syscall_mock.h"
#include "execute.h"
#include "command.h"
#include "builtin.h"
#include "strextra.h"

static void execute_command(scommand cmd);
static void redirect_input(scommand cmd);
static void redirect_output(scommand cmd);
static void execute_single_command(pipeline apipe);
static void execute_multiple_commands(pipeline apipe);

static void execute_command(scommand cmd) {
    assert(cmd != NULL);
    unsigned int length = scommand_length(cmd);

    if (builtin_is_internal(cmd)) {
        builtin_run(cmd);
        exit(EXIT_SUCCESS);
    } else {
        char **argument_list = calloc(length + 1, sizeof(char *));
        for (unsigned int i = 0; i < length; i++) {
            argument_list[i] = strdup(scommand_front(cmd));
            scommand_pop_front(cmd);
        }
        argument_list[length] = NULL;

        redirect_input(cmd);
        redirect_output(cmd);

        for (unsigned int i = 0; i <= length; i++) {
            if (execvp(argument_list[i], argument_list)) {
                perror("error");
            }
        }
    }
}

static void redirect_input(scommand cmd) {
    char *dir_in = scommand_get_redir_in(cmd);
    if (dir_in != NULL) {
        int fd_redir_in = open(dir_in, O_RDONLY, S_IWUSR);
        close(STDIN_FILENO);
        dup2(fd_redir_in, STDIN_FILENO);
        close(fd_redir_in);
    }
}

static void redirect_output(scommand cmd) {
    char *dir_out = scommand_get_redir_out(cmd);
    if (dir_out != NULL) {
        int fd_redir_out = open(dir_out, O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU);
        close(STDOUT_FILENO);
        dup2(fd_redir_out, STDOUT_FILENO);
        close(fd_redir_out);
    }
}

static void execute_single_command(pipeline apipe) {
    assert(apipe != NULL);
    char *invalid_cmd = "invalid comand";

    if (pipeline_is_empty(apipe)){
        write(STDOUT_FILENO, invalid_cmd, strlen(invalid_cmd));
    } else if (scommand_is_empty(pipeline_front(apipe))) {
        pipeline_pop_front(apipe);
    } else if (builtin_alone(apipe)){
        builtin_run(pipeline_front(apipe));
        pipeline_pop_front(apipe);
    }
}

static void execute_multiple_commands(pipeline apipe) {
    unsigned int length = pipeline_length(apipe);
    scommand cmd = pipeline_front(apipe);
    bool status_aux = pipeline_get_wait(apipe);
    pid_t pid1;
    pid_t pid2;
    int fd1[2];
    int status;

    if (length >= 2) {
        pipe(fd1);
    }

    pid1 = fork();

    if (pid1 == 0) {
        if (length >= 2) {
            close(fd1[0]);
            dup2(fd1[1], STDOUT_FILENO);
            close(fd1[1]);
        }

        execute_command(cmd);
    } else {
        if (length == 1) {
            if (status_aux) {
                waitpid(pid1, &status, WCONTINUED);
            }
        } else if (length >= 2) {
            close(fd1[1]);
            pipeline_pop_front(apipe);
            cmd = pipeline_front(apipe);
            status_aux = pipeline_get_wait(apipe);

            pid2 = fork();

            if (pid2 == 0) {
                dup2(fd1[0], STDIN_FILENO);
                close(fd1[0]);
                execute_command(cmd);
            } else {
                close(fd1[0]);
                if (length == 2 && status_aux) {
                    waitpid(pid1, &status, WCONTINUED);
                    waitpid(pid2, &status, WCONTINUED);
                }
            }
        }
    }
}

void execute_pipeline(pipeline apipe) {
    if (pipeline_is_empty(apipe)) {
        return;
    }

    if (builtin_alone(apipe)) {
        builtin_run(pipeline_front(apipe));
        pipeline_pop_front(apipe);
    } else {
        execute_single_command(apipe);
        execute_multiple_commands(apipe);
    }
}
