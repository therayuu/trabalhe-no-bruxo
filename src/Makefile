CFLAGS = -Wall -Wextra -I lib/
LFLAGS = -lm -lSDL2 -lSDL2_ttf -lSDL2_image

ifneq ($(shell command -v pkg-config;),)
    CFLAGS += $(shell pkg-config --cflags sdl2)
endif

all: main
main: main.exe

run: main.exe
	./main.exe

%.exe: %.c
	gcc $< -o $@ $(CFLAGS) $(LFLAGS)

.PHONY: run all main
