all: server/index.html server/cpkthttp

server/index.html: client/index.html
	./utils/deflate.py client/index.html server/index.html

server/cpkthttp: server/c/main.c
	clang -O2 -finstrument-functions -g -gdwarf-4 -o server/cpkthttp server/c/main.c

clean:
	rm server/index.html
	rm server/cpkthttp
