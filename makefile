result = compresor
main = main.c
CC=gcc
CFLAGS = -Wall -Wextra -g
OBJS = $(patsubst %.h, %.o, $(wildcard *.h))

$(result): $(main) $(OBJS)
	$(CC) $(CFLAGS) -o $(result) $(main) $(OBJS)

#compilar todos los objetos
%.o: %.c %.h
	$(CC) $(CFLAGS) -c -o $@ $<

.PHONY: clean leak

leak: $(result)
	valgrind --leak-check=full --show-leak-kinds=all ./$(result)

all: $(result)

clean:
	rm $(result) *.o *.temp
