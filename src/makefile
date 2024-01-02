INCLUDE = ../include
SRC_FILES = main.cpp glad.cpp stb_image_stub.cpp
CFLAGS = -Wall -std=c++11 -g -pedantic -Wno-strict-prototypes

OSX = -framework OpenGL -lglfw -I/opt/homebrew/include -L/opt/homebrew/lib

default: osx

osx: $(SRC_FILES)
	clang++ $(SRC_FILES) -o ../hello_triangle -I$(INCLUDE) $(OSX) $(CFLAGS)

clean:
	rm -rf *.o
	rm -rf ../*.dSYM