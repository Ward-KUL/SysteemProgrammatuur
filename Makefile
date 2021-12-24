test:
	gcc -g -o buffer -Wall -pthread sbuffer.c main.c
	./buffer

testc:
	clear
	make test