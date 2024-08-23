CC = clang
CFLAGS = -g -O0 -Wall -Wextra #-Wunused

all: game

OBJECTS = game.o solver.o main.o

game: $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o game.out

main.o: main.c
	$(CC) $(CFLAGS) -c main.c

game.o: game.c
	$(CC) $(CFLAGS) -c game.c

solver.o: solver.c
	$(CC) $(CFLAGS) -c solver.c

clean:
	rm -f game.out *.o

fmt:
	clang-format -i *.c *.h

.DEFAULT: all

