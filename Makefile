test:
	gcc -g -o buffer -Wall -Werror	 -pthread sbuffer.c main.c
	./buffer

val:
	make test
	valgrind --leak-check=full ./buffer


testc:
	clear
	make test
compile:
	gcc -g -o buffer -Wall -pthread sbuffer.c main.c

gdb:
	gcc -g -o buffer -Wall -pthread sbuffer.c main.c
	export CK_FORK=no;gdb -tui buffer