CC = gcc
CFLAGS = -Wall -Wextra

test:
	$(CC) $(CFLAGS) board.c player.c strat.c events.c finance.c game.c test.c -o test

main:
	$(CC) $(CFLAGS) board.c player.c strat.c events.c finance.c game.c main.c -o main

clean:
	rm -f test main

.PHONY: clean test main