CC=cc
CFLAGS=-g -Wall
DEPS=sockets-example.h
OBJ=main.o send.o recv.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

sockets-example: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

test:
	make && ./sockets-example

clean:
	rm sockets-example *.o
