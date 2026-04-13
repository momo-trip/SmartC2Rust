CC = clang
CFLAGS = -g -W -Wall -finstrument-functions -gdwarf-4 -Wno-nonnull -Wno-unused-parameter -Wno-implicit-function-declaration
LIBS = -lpthread #-lsocket

all: httpd client

httpd: httpd.c
	$(CC) $(CFLAGS) $(LIBS) -o $@ $<

client: simpleclient.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f httpd client