CC = clang
CFLAGS = -Wall -std=c99 -finstrument-functions -g -gdwarf-4
SRCDIR = src
INCLUDES = -I$(SRCDIR)

SRC_FILES = $(wildcard $(SRCDIR)/*.c)
OBJ_FILES = $(SRC_FILES:.c=.o)
TEST_OBJ = test.o

TARGET = test

all: $(TARGET)

$(TARGET): $(TEST_OBJ) $(OBJ_FILES)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(SRCDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -f $(TARGET) $(TEST_OBJ) $(OBJ_FILES)

.PHONY: all clean