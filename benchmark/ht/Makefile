CC = clang
CFLAGS = -Wall -O2 -finstrument-functions -g -gdwarf-4

all: lsearch bsearch demo dump stats # perfget-c perfset-c perflbh

# perfget-c: samples/perfget.c ht.c
# 	$(CC) $(CFLAGS) -o perfget-c samples/perfget.c ht.c

# perfset-c: samples/perfset.c ht.c
# 	$(CC) $(CFLAGS) -o perfset-c samples/perfset.c ht.c

# perflbh: samples/perflbh.c ht.c
# 	$(CC) -O2 -Wall -o perflbh samples/perflbh.c ht.c

lsearch: samples/lsearch.c
	$(CC) $(CFLAGS) -o lsearch samples/lsearch.c

bsearch: samples/bsearch.c
	$(CC) $(CFLAGS) -o bsearch samples/bsearch.c

demo: samples/demo.c ht.c
	$(CC) $(CFLAGS) -o demo samples/demo.c ht.c

dump: samples/dump.c ht.c
	$(CC) $(CFLAGS) -o dump samples/dump.c ht.c

stats: samples/stats.c ht.c
	$(CC) -O2 -Wall -o stats samples/stats.c ht.c

clean:
	rm -f perfget-c perfset-c perflbh lsearch bsearch demo dump stats

.PHONY: all clean