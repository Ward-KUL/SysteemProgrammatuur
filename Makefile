# Create your own targets that compile the application
test:
	gcc -g -o test_server -lpthread test_server.c lib/tcpsock.c
	gcc -g -o client -lpthread sensor_node.c lib/tcpsock.c


# the files for ex2 will be ziped and are then ready to
# be submitted to labtools.groept.be
zip:
	zip lab7_ex2.zip connmgr.c connmgr.h config.h lib/dplist.c lib/dplist.h lib/tcpsock.c lib/tcpsock.h
