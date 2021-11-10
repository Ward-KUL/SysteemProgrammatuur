HEADERS=dplist.h
SOURCE=dplist.c
EXE=build/dplist

#std used to be c11

FLAGS = -std=c11 -Werror -DCMAKE_TOOLCHAIN_FILE=C:/Users/Ward/vcpkg/scripts/buildsystems/vcpkg.cmake -lm $(shell pkg-config --cflags --libs check)

test : dplist.c dplist_test.c
	@echo -e '\n*******************************'
	@echo -e '*** Compiling for UNIT TEST ***'
	@echo -e '*******************************'
	mkdir -p build
	gcc dplist.c dplist_test.c -o build/dplist_test $(FLAGS)
	@echo -e '\n*************************'
	@echo -e '*** Running UNIT TEST ***'
	@echo -e '*************************'
	./build/dplist_test 

test_valgrind: dplist.c dplist_test.c
	@echo -e '\n*******************************'
	@echo -e '*** Compiling for UNIT TEST valgrind***'
	@echo -e '*******************************'
	mkdir -p build
	gcc dplist.c -g dplist_test.c -o build/dplist_test $(FLAGS)
	@echo -e '\n*************************'
	@echo -e '*** Running UNIT TEST valgrind ***'
	@echo -e '*************************'
	valgrind --leak-check=full ./build/dplist_test 

test_debug : dplist.c dplist_test.c
	@echo -e '\n*******************************'
	@echo -e '*** Compiling for UNIT TEST DEBUG***'
	@echo -e '*******************************'
	mkdir -p build
	gcc dplist.c dplist_test.c -o build/dplist_test $(FLAGS) -g
	@echo -e '\n*************************'
	@echo -e '*** Running UNIT TEST DEBUG***'
	@echo -e '*************************'
	export CK_FORK=no; gdb -q ./build/dplist_test

.PHONY : clean check

check : $(SOURCE) $(HEADERS)
	@echo "Running cppcheck :"
	cppcheck --enable=all --suppress=missingIncludeSystem --error-exitcode=1 $(SOURCE) $(HEADERS)

clean:
	rm -f -v !(*.c) !(*.h) !(Makefile)
