CC := clang
CFLAGS := -std=c99 -finstrument-functions -g -gdwarf-4
TARGET := bst

all: $(TARGET)

$(TARGET): bst.c
	$(CC) $(CFLAGS) $^ -o $@

clean:
	rm -f $(TARGET)

.PHONY: all clean