CFLAGS = -g -Wall -Werror -Wpedantic
CC = gcc

srcs = $(wildcard *.c)
objs = $(srcs:.c=.o)

all: p1.x p2.x p3.x

p1.x: $(objs)
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -rf *~ *.o *.x

.PHONY: all clean
