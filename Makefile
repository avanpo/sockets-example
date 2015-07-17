CC=cc
CFLAGS=-g -Wall
DEPS=
OBJ=main.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

sockets-example: main.o
	$(CC) -o $@ $^ $(CFLAGS)

test:
	make && ./sockets-example

clean:
	rm sockets-example *.o
