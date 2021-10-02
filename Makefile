# Create your own targets that build the dynamic library for the list and then compile the application


# the files for ex3 will be ziped and are then ready to
# be submitted to labtools.groept.be
zip:
	zip lab5_ex3.zip datamgr.c datamgr.h config.h lib/dplist.c lib/dplist.h
