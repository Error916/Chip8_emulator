OPT=-pedantic -Wall -Wextra -Ofast -flto -march=native -pipe
CFLAGS=-I/usr/include/SDL2 -D_REENTRANT
LIBS=-L/usr/lib -lSDL2
SRC=src/main.c src/chip8.c
CC=gcc

emu: $(SRC)
	$(CC) $(OPT) -o emu $(SRC) $(LIBS) $(CFLAGS)
