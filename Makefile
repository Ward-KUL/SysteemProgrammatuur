# Create your own targets that compile the application
test:
	gcc -g main.c sensor_db.c -o sensor_db.out -lsqlite3 -Wall -Werror -std=c99 -lm $(shell pkg-config --cflags --libs check)
	./sensor_db.out
gdb:
	gcc -g main.c sensor_db.c -o sensor_db.out -lsqlite3 -Wall -Werror -std=c99 -lm $(shell pkg-config --cflags --libs check)
	export CK_FORK=no; gdb -tui -q ./sensor_db.out

simple:
	gcc -g main.c sensor_db.c -o sensor_db.out -lsqlite3 -Wall -std=c99 -lm $(shell pkg-config --cflags --libs check)
	./sensor_db.out
val:
	gcc -g main.c sensor_db.c -o sensor_db.out -lsqlite3 -Wall -Werror -std=c99 -lm $(shell pkg-config --cflags --libs check)
	valgrind --leak-check=full --show-leak-kinds=all ./sensor_db.out
vals:
	gcc -g main.c sensor_db.c -o sensor_db.out -lsqlite3 -Wall -std=c99 -lm $(shell pkg-config --cflags --libs check)
	valgrind --leak-check=full ./sensor_db.out

tol:
	ccpcheck

# the files for ex2 will be ziped and are then ready to
# be submitted to labtools.groept.be
zip:
	zip lab6_ex2.zip sensor_db.c config.h
