all: parse.o shell.o builtin.o run_command.o shell

shell: shell.o parse.o
	gcc shell.o parse.o builtin.o run_command.o -o shell

shell.o: shell.c shell.h
	gcc -c shell.c

parse.o: parse.c shell.h
	gcc -c parse.c

builtin.o: builtin.c shell.h
	gcc -c builtin.c

run_command.o: run_command.c shell.h
	gcc -c run_command.c

clean:
	rm -f *.o shell
