#include <sys/types.h>
#include <unistd.h>
#include <string.h>

#include "utils.h"

ssize_t print (char * msg) {
    int size = strlen(msg);

    return write(STDOUT_FILENO, msg, size);
}

ssize_t print_err (char * msg) {
    int size = strlen(msg);

    return write(STDERR_FILENO, msg, size);
}
