# Compiler settings
CC = gcc
#CFLAGS = -Wall -Wextra -O2 -Wno-maybe-uninitialized -Wno-format -Wno-parentheses -Wno-main
CFLAGS = -Wall -Wextra -O2 -Wno-maybe-uninitialized -Wno-format -Wno-parentheses -Wno-main -ffreestanding -finstrument-functions -g -gdwarf-4

# Target executable name
TARGET = c4

# Default target and rule to build the executable
all $(TARGET): c4.c; $(CC) $(CFLAGS) -o $(TARGET) $<

# Clean target
clean: ; rm -f $(TARGET) *.o

.PHONY: all clean