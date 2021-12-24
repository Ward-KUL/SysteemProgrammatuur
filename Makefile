test:
	gcc -g -o buffer -Wall -pthread sbuffer.c main.c
	./buffer

testc:
	clear
	make test

gdb:
	gcc -g -o buffer -Wall -pthread sbuffer.c main.c
	export CK_FORK=no;gdb -tui buffer