CC = clang
CFLAGS = -Wall -std=c99 -pedantic -O2 -finstrument-functions -g -gdwarf-4

all: sds-test

sds-test: sds.c sds.h testhelp.h sdsalloc.h
#	$(CC) -o sds-test sds.c -Wall -std=c99 -pedantic -O2 -DSDS_TEST_MAIN
	$(CC) $(CFLAGS) -o sds-test sds.c -DSDS_TEST_MAIN
	@echo ">>> Type ./sds-test to run the sds.c unit tests."

clean: 
	rm -f sds-test
