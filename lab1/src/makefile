
GXX=g++
SDL2_CFLAGS=`sdl2-config --cflags`
SDL2_LIBS=`sdl2-config --libs`
GL_LIBS=-lGL -lGLU
CFLAGS=$(SDL2_CFLAGS)
LDFLAGS=$(SDL2_LIBS) $(GL_LIBS) -ldl
GLAD=-I/home/nikola/Dev/faks/racunalna_grafika/lab1/include/

.PHONY: clean

all: bspline

bspline: main.o Shader.o Object.o BSpline.o glad.o
	$(GXX) -o bspline main.o Shader.o Object.o BSpline.o glad.o $(LDFLAGS)

main.o: main.cpp tiny_obj_loader.h Shader.h Object.h BSpline.h glad.o
	$(GXX) $(GLAD) $(CFLAGS) -c main.cpp

Shader.o: Shader.cpp Shader.h glad.o
	$(GXX) $(GLAD) -c Shader.cpp

Object.o: glad.o tiny_obj_loader.h Object.h Object.cpp
	$(GXX) $(GLAD) -c Object.cpp

BSpline.o: BSpline.h BSpline.cpp
	$(GXX) $(GLAD) -c BSpline.cpp

glad.o: glad.c
	$(GXX) $(GLAD) -c glad.c

clean:
	rm bspline *.o
