result = compresor
main = main.c
CC=gcc
CFLAGS = -Wall -Wextra
OBJS = $(patsubst %.h, %.o, $(wildcard *.h))

$(result): $(main) $(OBJS)
	$(CC) $(CFLAGS) -o $(result) $(main) $(OBJS)

#compilar todos los objetos
%.o: %.c %.h
	$(CC) $(CFLAGS) -c -o $@ $<

.PHONY: clean run leak

run: $(result)
	./$(result)

leak: $(result)
	valgrind --leak-check=full --show-leak-kinds=all ./$(result)

clean:
	rm $(result) $(OBJS) *.temp
