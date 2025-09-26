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

.PHONY: clean massif leak

leak: $(result)
	valgrind --leak-check=full --show-leak-kinds=all ./$(result) $(ARGS)

massif: $(result)
	rm massif.out.*
	valgrind --tool=massif --stacks=yes --detailed-freq=1 --max-snapshots=1000 --alloc-fn=resize_list ./$(result) $(ARGS)
	massif-visualizer massif.out.*

all: $(result)

clean:
	rm $(result) *.o *.temp massif.out.*
