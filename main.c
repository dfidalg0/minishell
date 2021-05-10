#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdbool.h>

#include "utils.h"
#include "parser.h"
#include "command.h"

int main() {
    // Entrada no programa
    print("Welcome to the minishell (type exit to leave)\n");

    // Variável para armazenamento do comando completo
    char input[5000];

    while (true) {
        // Indicador para o usuário que este pode digitar
        print("$ ");

        // Saída do programa pelo EOF
        if (fgets(input, 5000, stdin) <= 0) {
            print("exit\n");
            break;
        }

        Command * cmd = parse(input);

        if (cmd) {
            spawn(cmd);

            free_command(cmd);
        }
    }

    return 0;
}
