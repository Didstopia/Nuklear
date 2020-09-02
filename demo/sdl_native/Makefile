# Install
BIN = demo

# Flags
CFLAGS += -std=c99 -pedantic -O2 -w

SRC = main.c
OBJ = $(SRC:.c=.o)

LIBLDIR = ../../dependencies/SDL2_gfx-1.0.4

ifeq ($(OS),Windows_NT)
BIN := $(BIN).exe
LIBS = -lmingw32 -lSDL2main -lSDL2 -lSDL2_ttf -lSDL2_gfx -lm
else
	UNAME_S := $(shell uname -s)
	ifeq ($(UNAME_S),Darwin)
		LIBS = -lSDL2 -lSDL2_ttf -lSDL2_gfx -framework OpenGL -framework Metal -lm
	else
		LIBS = `sdl2-config --libs` -lSDL2_ttf -lSDL2_gfx -lm
	endif
endif

$(BIN):
	@mkdir -p bin
	rm -f bin/$(BIN) $(OBJS)
	$(CC) $(SRC) $(CFLAGS) -o bin/$(BIN) -L$(LIBLDIR) $(LIBS)
