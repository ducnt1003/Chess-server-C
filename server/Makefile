CC = gcc -std=c99
CFLAGS=-Wall -g -fPIE -Wno-format-overflow

all: chess_console

OBJS = gameserver.o gameserver_core.o users.o message.o rooms.o

chess_console: $(OBJS)
	$(CC) -o server $(OBJS)

.SUFFIXES: .c .o
.c.o:
	$(CC) $(CFLAGS) -c $*.c

clean:
	rm -f *.o server
