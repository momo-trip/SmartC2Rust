CC := clang
CFLAGS := -std=c99 -finstrument-functions -g -gdwarf-4

test: test.c src/rgba.c
	$(CC) $(CFLAGS) $^ -o $@
#	@./test

.PHONY: test