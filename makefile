result = compresor
main = main.c
CC=gcc
CFLAGS = -Wall -Wextra -O0 -g
OBJS = $(patsubst %.h, %.o, $(wildcard *.h))
type=$(result)

$(result): $(main) $(OBJS)
	$(CC) $(CFLAGS) -o $(result) $(main) $(OBJS) -lm

fast: CFLAGS = -O3
fast: $(result)

#compilar todos los objetos
%.o: %.c %.h makefile
	$(CC) $(CFLAGS) -c -o $@ $<

.PHONY: clean massif leak run all final

run: $(type)
	./$(result) $(ARGS)

leak: $(type)
	valgrind --leak-check=full --show-leak-kinds=all ./$(result) $(ARGS)

massif: $(type)
	rm massif.out.* || true
	valgrind --tool=massif --stacks=yes --detailed-freq=1 --max-snapshots=1000 --alloc-fn=resize_list ./$(result) $(ARGS)
	massif-visualizer massif.out.*

all: $(type)

clean:
	rm $(result) *.o *.temp massif.out.* || true
