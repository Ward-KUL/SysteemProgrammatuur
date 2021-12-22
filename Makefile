# Create your own targets that compile the application
test:
	gcc -g -o test_server -lpthread test_server.c lib/tcpsock.c lib/dplist.c
	gcc -g -o client -lpthread sensor_node.c lib/tcpsock.c
server:
	make test
	./test_server
test_orig:
	gcc -g -o test_server -lpthread test_server_orig.c lib/tcpsock.c lib/dplist.c
	gcc -g -o client -lpthread sensor_node.c lib/tcpsock.c
server_o:
	make test_orig
	./test_server

client:
	make client
	./client 1 1 127.0.0.1 5678
servergdb:
	make test
	export CK_FORK=no;gdb -tui ./test_server


# the files for ex2 will be ziped and are then ready to
# be submitted to labtools.groept.be
zip:
	zip lab7_ex2.zip connmgr.c connmgr.h config.h lib/dplist.c lib/dplist.h lib/tcpsock.c lib/tcpsock.h
