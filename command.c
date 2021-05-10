#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <wait.h>
#include <fcntl.h>

#include "utils.h"
#include "command.h"

void __spawn_recursion(Command * cmd, int * prevpipe);

// Cria um novo comando preenchido com valores base.
Command *create_command() {
    Command * cmd = malloc(sizeof(Command));

    // Arquivos de entrada e saída
    cmd->io.fin = NULL;
    cmd->io.fout = NULL;

    // De início, não existe um PIPE
    cmd->next = NULL;

    // Começamos com argv de tamanho 10, podendo expandí-lo depois
    cmd->_max_args = 10;
    // O bloco de memória alocado, no entanto, deve ser de tamanho 11,
    // pois é necessário ter o valor NULL após o último argumento
    cmd->argv = calloc(sizeof(char *), 1 + cmd->_max_args);

    return cmd;
}

// Limpa TODA a memória usada por um Command
void free_command(Command * cmd) {
    // Recursão de limpeza da lista ligada
    if (cmd->next) {
        free_command(cmd->next);
    }

    char ** arg = cmd->argv; // Ponteiro para auxiliar a limpeza do argv

    // Limpeza de cada string em argv
    while (*arg != NULL) {
        free(*(arg++));
    }

    // Limpeza do argv
    free(cmd->argv);

    // Limpeza dos nomes dos arquivos de entrada e saída
    if (cmd->io.fin ) free(cmd->io.fin );
    if (cmd->io.fout) free(cmd->io.fout);

    // Limpeza do comando
    free(cmd);
}

// Expande o número máximo de argumentos que um comando pode alocar
int expand_args (Command * cmd, int n_args) {
    if (n_args < cmd->_max_args) {
        print_err("O novo tamanho de argv deve ser maior que o anterior\n");
        return -1;
    }

    // Realocação de memória
    char ** _new = realloc(cmd->argv, (1 + n_args) * sizeof(char *));

    if (!_new) {
        return -1;
    }

    cmd->argv = _new;

    // Preenchimento dos novos elementos de argv com NULL
    for (int i = 1 + cmd->_max_args; i <= n_args; ++i) {
        cmd->argv[i] = NULL;
    }

    // Atualização de _max_args
    cmd->_max_args = n_args;
}

// Execução de um comando
int spawn(Command * cmd) {
    // Saída do shell
    if (!cmd->next && !strcmp("exit", cmd->argv[0])) {
        exit(0);
    }

    // Recursão interna do processo de execução
    __spawn_recursion(cmd, NULL);

    // Espera pela finalização de todos os processos filhos.
    while (wait(NULL) > 0);

    return 0;
}

// Recursão interna do processo de execução
void __spawn_recursion(Command * cmd, int * prevpipe) {
    int pipefd[2] = {}; // PIPE para o próximo processo

    if (cmd->next) {
        pipe(pipefd); // Preenchimento do PIPE
    }

    // Código a ser executado no processo pai (shell)
    if (fork() != 0) {
        // Antes de continuar a recursão, o shell deve fechar o PIPE que não
        // será mais necessário
        if (prevpipe) {
            close(prevpipe[0]);
            close(prevpipe[1]);
        }

        // Se houver outro processo para um PIPE, ele deverá ser executado
        if (cmd->next) {
            __spawn_recursion(cmd->next, pipefd);
        }

        return;
    }

    // fds de entrada e saída do processo
    struct {
        int in;
        int out;
    } fd = { 0, 0 };

    // Abertura do fd de entrada
    if (cmd->io.fin) {
        fd.in = open(cmd->io.fin, O_RDONLY);

        if (fd.in == -1) {
            print_err(cmd->io.fin);
            print_err(": Arquivo não encontrado\n");
            exit(1);
        }
    }

    // Abertura do fd de saída
    if (cmd->io.fout) {
        fd.out = open(cmd->io.fout, O_WRONLY | O_CREAT | O_TRUNC, 0664);

        if (fd.out == -1) {
            print_err(cmd->io.fout);
            print_err(": Arquivo não encontrado\n");
            exit(1);
        }
    }

    // Convenção: Os operadores > e < têm precedência sobre |

    // Modificação da entrada padrão
    if (fd.in) {
        dup2(fd.in, STDIN_FILENO);
        if (prevpipe) close(prevpipe[0]); // o PIPE não será usado
    }
    else if (prevpipe)
        dup2(prevpipe[0], STDIN_FILENO);

    // Modificação da saída padrão
    if (fd.out){
        dup2(fd.out, STDOUT_FILENO);
        if (pipefd[0]) close(pipefd[1]); // o PIPE não será usado
    }
    else if (pipefd[0])
        dup2(pipefd[1], STDOUT_FILENO);

    // A extremidade de saída do PIPE anterior não será necessária
    if (prevpipe) close(prevpipe[1]);

    // Bem como a extrenmidade de entrada do próximo PIPE
    if (pipefd[0]) close(pipefd[0]);

    // Por ora, não é necessário resolver variáveis ambiente
    char ** envp = { NULL };

    // Substituição da imagem de execução
    execve(cmd->argv[0], cmd->argv, envp);

    // Rota alternativa caso o execve falhe
    print_err(cmd->argv[0]);
    print_err(": Comando não encontrado\n");

    exit(1);
}
