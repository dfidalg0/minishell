#ifndef __COMMAND__
#define __COMMAND__

typedef struct Command {
    char** argv;
    int _max_args;
    struct {
        int in;
        int out;
    } fd;
    struct Command * next;
} Command;

Command * create_command();
void free_command(Command *);

int expand_args(Command *, int max_args);

int spawn(Command *);

#endif // __COMMAND__
