#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <wait.h>

#include "utils.h"
#include "command.h"

int __spawn_recursion(Command * cmd, int * prevpipe);

Command *create_command() {
    Command * cmd = malloc(sizeof(Command));

    cmd->fd.in = STDIN_FILENO;
    cmd->fd.out = STDOUT_FILENO;
    cmd->next = NULL;
    cmd->_max_args = 10;
    cmd->argv = calloc(sizeof(char *), 1 + cmd->_max_args);

    return cmd;
}

void free_command(Command * cmd) {
    if (cmd->next) {
        free_command(cmd->next);
    }

    char ** arg = cmd->argv;

    while (*arg != NULL) {
        free(*(arg++));
    }

    free(cmd->argv);

    free(cmd);
}

int expand_args (Command * cmd, int n_args) {
    if (n_args < cmd->_max_args) {
        return -1;
    }

    char ** _new = realloc(cmd->argv, (1 + n_args) * sizeof(char *));

    if (!_new) {
        return -1;
    }

    cmd->argv = _new;

    for (int i = 1 + cmd->_max_args; i <= n_args; ++i) {
        cmd->argv[i] = NULL;
    }

    cmd->_max_args = n_args;
}

int spawn(Command * cmd) {
    if (!cmd->next && strcmp("exit", cmd->argv[0]) == 0) {
        exit(0);
    }

    __spawn_recursion(cmd, NULL);

    // Espera pela finalização de todos os processos filhos.
    while (wait(NULL) > 0);

    return 0;
}

int __spawn_recursion(Command * cmd, int * prevpipe) {
    bool r_in = cmd->fd.in != STDIN_FILENO;
    bool r_out = cmd->fd.out != STDOUT_FILENO;

    int pipefd[2] = {};

    if (cmd->next) {
        pipe(pipefd);
    }

    if (fork() == 0) {
        char ** envp = { NULL };

        if (r_in)
            dup2(cmd->fd.in, STDIN_FILENO);
        else if (prevpipe)
            dup2(prevpipe[0], STDIN_FILENO);

        if (r_out)
            dup2(cmd->fd.out, STDOUT_FILENO);
        else if (pipefd[0])
            dup2(pipefd[1], STDOUT_FILENO);

        if (prevpipe) close(prevpipe[1]);

        if (pipefd[0]) close(pipefd[0]);

        execve(cmd->argv[0], cmd->argv, envp);

        print_err(cmd->argv[0]);
        print_err(": Comando não encontrado\n");

        exit(1);
    }
    else {
        if (r_in) close(cmd->fd.in);

        if (r_out) close(cmd->fd.out);

        if (prevpipe) {
            close(prevpipe[0]);
            close(prevpipe[1]);
        }

        if (pipefd[0]) {
            __spawn_recursion(cmd->next, pipefd);
        }
    }
}
