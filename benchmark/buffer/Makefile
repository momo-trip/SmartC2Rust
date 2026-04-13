CC := clang
CFLAGS := -std=c99 -finstrument-functions -g -gdwarf-4
TARGET := test

all: $(TARGET)

$(TARGET): buffer.c test.c
	$(CC) $(CFLAGS) $^ -o $@

.PHONY: all clean

clean:
	@rm -f $(TARGET)