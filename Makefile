CC=gcc
CFLAGS=-g -pedantic -std=gnu17 -Wall -Werror -Wextra

.PHONY: all
all: nyush

nyush: nyush.c cmdparser.c execute.c utils.c cmdparser.h execute.h utils.h global.h
	gcc -o nyush nyush.c cmdparser.c execute.c utils.c

.PHONY: clean
clean:
	rm -f *.o nyush
