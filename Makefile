CC := gcc -std=gnu11

CFLAGS := -Wall -O2
LDFLAGS := -lm

SRC := main.c gif.c
BIN := asplashgen.exe

all:
	$(CC) $(CFLAGS) $(SRC) -o $(BIN) $(LDFLAGS)

debug:
	$(CC) $(CFLAGS) -O0 -g3 -ggdb -DDEBUG -o dbg_$(BIN) $(SRC) $(LDFLAGS)

clean:
	rm -f $(BIN) dbg_$(BIN)
