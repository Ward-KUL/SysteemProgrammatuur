# Create your own targets that build the dynamic library for the list and then compile the application

static:
	gcc -c lib/dplist.c -o libdplist.a
	gcc -static datamgr.c main.c -DSET_MAX_TEMP=20 -DSET_MIN_TEMP=10 libdplist.a -o datamgr

#-l zoekt achter de naam dat achter de l staat en zet er lib voor.
shared:
	gcc -fPIC -c lib/dplist.c
	gcc --shared -o libdplist.so dplist.o
	gcc -g main.c datamgr.c -L./ -Wl,-rpath=./ -ldplist -Wall -Werror -DSET_MAX_TEMP=20 -DSET_MIN_TEMP=10 -o datamgr_shared -lm $(shell pkg-config --cflags --libs check)
	./datamgr_shared
test:
	gcc -fPIC -c lib/dplist.c
	gcc --shared -o libdplist.so dplist.o
	gcc -g main.c datamgr.c -L./ -Wl,-rpath=./ -ldplist -Wall -Werror -DSET_MAX_TEMP=25 -DSET_MIN_TEMP=10 -o datamgr_shared -lm $(shell pkg-config --cflags --libs check)
	./datamgr_shared

shared_gdb:
	gcc -fPIC -c lib/dplist.c
	gcc --shared -o libdplist.so dplist.o
	gcc -g main.c datamgr.c -L./ -Wl,-rpath=./ -ldplist -Wall -Werror -DSET_MAX_TEMP=20 -DSET_MIN_TEMP=10 -o datamgr_shared -lm $(shell pkg-config --cflags --libs check)
	export CK_FORK=no; gdb -tui -q ./datamgr_shared
val:
	gcc -fPIC -c lib/dplist.c
	gcc --shared -o libdplist.so dplist.o
	gcc -g main.c datamgr.c -L./ -Wl,-rpath=./ -ldplist -Wall -Werror -DSET_MAX_TEMP=20 -DSET_MIN_TEMP=10 -o datamgr_shared -lm $(shell pkg-config --cflags --libs check)
	valgrind --leak-check=full ./datamgr_shared

labtool:
	gcc datamgr.c main.c -o datamgr_test -Wall -std=c11 -Werror -lm -DSET_MIN_TEMP=14.01 -DSET_MAX_TEMP=27.96 -L. -Wl,-rpath=. -ldplist -pthread -I/usr/local/include -L/usr/local/lib -lcheck
	


# the files for ex3 will be ziped and are then ready to
# be submitted to labtools.groept.be
	

zip:
	zip lab5_ex3.zip datamgr.c datamgr.h config.h lib/dplist.c lib/dplist.h
