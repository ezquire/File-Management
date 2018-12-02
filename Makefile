CFLAGS = -g -Wall -Werror -Wpedantic -lm
CC = gcc

srcs = $(wildcard *.c)
objs = $(srcs:.c=.o)

all: p1.x p3.x

p1.x: p1.o
	$(CC) $(CFLAGS) -o $@ $^

p3.x: p3.o
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -rf *~ *.o *.x

.PHONY: all clean
