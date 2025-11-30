LIBS = $(wildcard lib/*.h) $(wildcard lib/*.c)
CORE = $(wildcard *.c)

CFLAGS = -Wall -Wextra --std=c23 -I lib/
LFLAGS = -lm -lSDL2 -lSDL2_ttf -lSDL2_image

ifneq ($(shell command -v pkg-config;),)
    CFLAGS += $(shell pkg-config --cflags sdl2)
endif

all: main
main: main.exe

run: main.exe
	./main.exe

main.exe: $(LIBS) $(CORE)

%.exe: %.c
	gcc $< -o $@ $(CFLAGS) $(LFLAGS)

clean:
	-rm *.exe

.PHONY: run all main clean
