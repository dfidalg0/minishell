all:
	(test -d bin || mkdir bin) && gcc -o bin/minishell *.c
