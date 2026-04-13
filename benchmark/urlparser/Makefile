CC = clang
CFLAGS = -std=c99 -Wall -finstrument-functions -g -gdwarf-4 -Wno-unused-but-set-variable

all: clean test

clean:
	rm -rf url-test

test: test.c
	$(CC) $(CFLAGS) -I. $< -o url-test
#	$(CC) -std=c99 -Wall -I. $< -o url-test
#	./url-test

.PHONY: all clean test
