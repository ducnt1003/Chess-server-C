CC = gcc -std=c99
CFLAGS=-Wall -g -fPIE -Wno-deprecated-declarations -Wno-cpp

all: chess_console

OBJS = mainConsoleChess.o ClientHandle.o Client.o message.o tscp.a

chess_console: $(OBJS)
	$(CC) -o client $(OBJS)

.SUFFIXES: .c .o
.c.o:
	$(CC) $(CFLAGS) -c $*.c

clean:
	rm -f *.o client
