CC = clang
CFLAGS = -ggdb -std=gnu99 -finstrument-functions -g -gdwarf-4
BIN = markandsweep

.PHONY : clean 

$(BIN) : main.c
	$(CC) $(CFLAGS) main.c -o $(BIN)

clean :
	rm -f $(BIN) *~

run : $(BIN)
	valgrind  --leak-check=yes ./$(BIN)

