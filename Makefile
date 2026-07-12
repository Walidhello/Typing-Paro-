PROJECT = TypingParo

CC = gcc

CFLAGS = -Wall -std=c99 -Iinclude

LDFLAGS = -lraylib -lopengl32 -lgdi32 -lwinmm

SRC = src/main.c \
      src/game.c \
      src/enemy.c \
      src/player.c \
	  src/word.c

$(PROJECT).exe: $(SRC)
	$(CC) $(SRC) -o $(PROJECT).exe $(CFLAGS) $(LDFLAGS)

run: $(PROJECT).exe
	.\$(PROJECT).exe

clean:
	del /Q *.exe