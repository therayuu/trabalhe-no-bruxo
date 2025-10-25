CFLAGS = -Wall -Wextra -I lib/
LFLAGS = -lm -lSDL2 -lSDL2_ttf -lSDL2_image

ifneq ($(shell command -v pkg-config;),)
    CFLAGS += $(shell pkg-config --cflags sdl2)
endif

all: main
main: main.exe

run: main.exe
	./main.exe

main.exe: lib/AUX.h lib/cores.h menu.c

%.exe: %.c
	gcc $< -o $@ $(CFLAGS) $(LFLAGS)

.PHONY: run all main
