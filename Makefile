ex1: main.c
	mkdir -p build
	gcc -Wall -Werror -o ./build/ex1 main.c
	./build/ex1

zip: main.c
	zip lab1_ex1.zip main.c
