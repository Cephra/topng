CFLAGS += -std=c99 -Wall -g $(shell pkg-config --libs --cflags libpng) -lm

all: main

clean:
	rm -f main
