INCLUDE = ../include
SRC_FILES = main.c glad.c
CFLAGS = -Wall -std=c99 -g -pedantic -Wno-strict-prototypes

OSX = -framework OpenGL -lglfw -I/opt/homebrew/include -L/opt/homebrew/lib

default: osx

osx: $(SRC_FILES)
	gcc $(SRC_FILES) -o ../hello_triangle -I$(INCLUDE) $(OSX) $(CFLAGS)

clean:
	rm -rf *.o
	rm -rf ../*.dSYM