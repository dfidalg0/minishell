#ifndef __COMMAND__
#define __COMMAND__

// Struct usada para sistematizar as informações de um comando
typedef struct Command {
    char** argv; // Lista de argumentos do programa
    int _max_args; // Contador do limite de argumentos
    struct IO {
        char * fin;  // Nome do arquivo que substituirá a entrada padrão
        char * fout; // Nome do arquivo que substituirá a saída padrão
    } io;
    struct Command * next; // Ponteiro para o próximo comando na fila do PIPE
} Command;

Command * create_command();
void free_command(Command *);

int expand_args(Command *, int max_args);

int spawn(Command *);

#endif // __COMMAND__
