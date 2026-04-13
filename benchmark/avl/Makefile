CC := clang
CFLAGS := -std=c99 -finstrument-functions -g -gdwarf-4
TARGET := avl

all: $(TARGET)

$(TARGET): avl.c
	$(CC) $(CFLAGS) $^ -o $@

.PHONY: all