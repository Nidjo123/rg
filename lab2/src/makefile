
GXX=g++ -g
SDL2_CFLAGS=`sdl2-config --cflags`
SDL2_LIBS=`sdl2-config --libs`
GL_LIBS=-lGL -lGLU
CFLAGS=$(SDL2_CFLAGS)
LDFLAGS=$(SDL2_LIBS) $(GL_LIBS) -ldl
GLAD=-I/home/nikola/Dev/faks/racunalna_grafika/lab1/include/

.PHONY: clean

all: particles

particles: main.o Shader.o Object.o glad.o Camera.o Particle.o stb_image.o
	$(GXX) -o particles main.o Shader.o Object.o Camera.o Particle.o glad.o stb_image.o $(LDFLAGS)

main.o: main.cpp tiny_obj_loader.h Shader.h Object.h Camera.h Particle.o glad.o
	$(GXX) $(GLAD) $(CFLAGS) -c main.cpp

Shader.o: Shader.cpp Shader.h glad.o
	$(GXX) $(GLAD) -c Shader.cpp

Object.o: glad.o tiny_obj_loader.h Object.h Object.cpp
	$(GXX) $(GLAD) -c Object.cpp

Camera.o: Camera.h Camera.cpp
	$(GXX) -c Camera.cpp

Particle.o: Particle.h
	$(GXX) -c Particle.cpp

glad.o: glad.c
	$(GXX) $(GLAD) -c glad.c

stb_image.o: stb_image.h stb_image.cpp
	$(GXX) -c stb_image.cpp

clean:
	rm particles *.o
