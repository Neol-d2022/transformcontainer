CC=gcc

CFLAGS=-Wall -Wextra -Ofast -D_TC_PRODUCTION
LFLAGS=-s

OBJS=main.o mm.o transformcontainer.o
DEPS=mm.h transformcontainer.h
LIBS=

BIN=test

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

$(BIN): $(OBJS)
	$(CC) -o $@ $^ $(LFLAGS) $(LIBS)

clean:
	rm -f $(OBJS) $(BIN)
