#include <stdbool.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

#include "parser.h"
#include "command.h"
#include "utils.h"

/* Declaração de estruturas e funções auxiliares */

// Struct usada para leitura de um único argumento de um comando
typedef struct __argument {
    char* value; // resultado em texto para o argumento
    int n_read;  // número de caracteres lidos do input
} __argument;

__argument * __create_arg();               // Criação de argumento
__argument * __read_arg(char* begin);      // Leitura de um argumento a partir de um trecho do input
bool __is_arg_end(char c, bool quotes_on); // Função auxiliar para detectar o fim da leitura

/* Implementação da função principal exposta para uso */
Command * parse(char * input) {
    Command * root = create_command(); // Comando principal da execução
    Command * curr = root;             // Ponteiro auxiliar para percorrer a lista
    ReadMode mode = R_ARG;             // Modo de leitura atual

    int argc = 0;  // Contador de argumentos lidos

    char * p = input;  // Ponteiro auxiliar para percorrer o input

    // Loop de leitura
    while (*p != '\n' && *p != '\0') {
        while (*p == ' ') ++p; // Espaços sobrando devem ser ignorados

        // Situações de leitura especiais
        switch (*p) {
        case '|':
            // Indica o fim da leitura de um comando, mudando para o próximo
            argc = 0;
            curr = curr->next = create_command();
            ++p;
            continue;
        case '>':
            // Indica que o próximo argumento lido será o nome do arquivo de saída
            mode = R_OUT;
            ++p;
            continue;
        case '<':
            // Indica que o próximo argumento lido será o nome do arquivo de entrada
            mode = R_INP;
            ++p;
            continue;
        }

        // Leitura do argumento
        __argument * arg = __read_arg(p);

        // Erro de leitura do argumento
        if (!arg) {
            free_command(root);

            return NULL;
        }

        // int fd; // File descriptor a ser usado para leitura e escrita

        switch (mode) {
        case R_ARG:
            // Expansão do número máximo de argumentos caso necessário.
            if (
                argc + 1 > curr->_max_args &&
                expand_args(curr, curr->_max_args + 10) == -1
            ) {
                print_err("Erro interno de memória\n");

                free_command(root);

                return NULL;
            }

            // Preenchimento do argv
            curr->argv[argc++] = arg->value;
            break;
        case R_INP:
            // Preenchimento do arquivo de entrada
            curr->io.fin = arg->value;
            break;
        case R_OUT:
            // Preenchimnto do arquivo de saída
            curr->io.fout = arg->value;
            break;
        }

        // Após a leitura de todo argumento, o modo de leitura padrão será sempre R_ARG
        mode = R_ARG;

        // Atualização do ponteiro p conforme o número de caracteres já lidos.
        p += arg->n_read;
    }

    return root;
}

/* Implementação das funções auxiliares */

// Criação de um argumento base
__argument* __create_arg() {
    __argument* arg = malloc(sizeof(__argument));
    arg->n_read = 0;
    arg->value = NULL;

    return arg;
}

// Leitura de um argumento
__argument * __read_arg(char* begin) {
    __argument * arg = __create_arg(); // Valor de retorno

    int n_quotes = 0; // Contador de aspas

    char* p = begin; // Ponteiro auxiliar de leitura

    // Determinação da posição do fim do argumento na string
    for (/*p = cmd*/; !__is_arg_end(*p, (n_quotes & 1) == 1); ++p) {
        if (*p == '"') {
            ++n_quotes;
        }
    }

    char * end = p;

    // Se o número de aspas for ímpar, alguma delas não foi devidamente fechada
    // Se o final da string for igual ao seu início, p nunca foi incrementado
    if (n_quotes & 1 == 1 || end <= begin) {
        print_err("Fim do argumento inesperado\n");
        return NULL;
    }

    // Determinação do número de bytes lidos
    arg->n_read = (int)(end - begin);

    // Tamanho real do argumento
    int len = arg->n_read - n_quotes;

    // Alocação do valor do argumento no tamanho necessário
    arg->value = malloc((1 + len) * sizeof(char));

    // Preenchimento do argumento com os caracteres necessários
    int i = 0;

    for (char * p = begin; p != end; ++p) {
        // Aspas são ignoradas
        if (*p != '"')
            arg->value[i++] = *p;
    }

    // Finalização da string
    arg->value[len] = '\0';

    return arg;
}

bool __is_arg_end(char c, bool quotes_on) {
    // Com aspas abertas, caracteres |, >, < ou espaços são permitidos
    if (quotes_on) {
        return c == '\0' || c == '\n';
    }

    // Caso contrário, esses caracteres indicam o fim da leitura do argumento
    return c == ' ' || c == '\n' || c == '|' || c == '>' || c == '<' || c == '\0';
}
