#ifndef __PARSER__
#define __PARSER__

#include "command.h"

// Modo de leitura
typedef enum ReadMode {
    R_ARG, // Leitura de argumento
    R_INP, // Leitura do nome do arquivo de entrada
    R_OUT  // Leitura do nome do arquivo de saída
} ReadMode;

Command * parse(char * input);

#endif // __PARSER__
