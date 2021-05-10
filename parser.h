#ifndef __PARSER__
#define __PARSER__

#include "command.h"

typedef enum ReadMode {
    R_ARG, R_INP, R_OUT
} ReadMode;

Command * parse(char * input);

#endif // __PARSER__
