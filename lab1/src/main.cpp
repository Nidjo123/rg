#include <glad/glad.h>

#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>

#define TINYOBJLOADER_IMPLEMENTATION // define this in only *one* .cc
#include "tiny_obj_loader.h"

#define DEBUG 1

static const char *NAME = "RG -- B-Spline";
static const int WIDTH = 800;
static const int HEIGHT = 600;

void print_debug_info() {
  SDL_version compiled;
  SDL_version linked;

  SDL_VERSION(&compiled);
  SDL_GetVersion(&linked);

  SDL_Log("Compiled against SDL version %d.%d.%d.\n",
	  compiled.major, compiled.minor, compiled.patch);
  SDL_Log("Linked against SDL version %d.%d.%d.\n",
	  linked.major, linked.minor, linked.patch);

  SDL_Log("Platform: %s\n", SDL_GetPlatform());
  SDL_Log("CPU: %d logical cores\n", SDL_GetCPUCount());
  SDL_Log("RAM: %d MB\n", SDL_GetSystemRAM());
}

int main(int argc, char *argv[]) {
  std::string inputfile = "obj/frog.obj";
  tinyobj::attrib_t attrib;
  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;

  std::string err;
  std::string warn;
  bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, "../obj/ArabianCity.obj", "../obj");

  if (!warn.empty()) {
    std::cout << warn << std::endl;
  }

  if (!err.empty()) { // `err` may contain warning message.
    std::cerr << err << std::endl;
  }

  if (!ret) {
    exit(1);
  }

  std::cout << "Successfully loaded " << inputfile << "!\n";



  SDL_Window *window = NULL;

  SDL_SetMainReady();
  SDL_Init(SDL_INIT_VIDEO  | SDL_INIT_AUDIO);

  if (DEBUG) {
    SDL_LogSetAllPriority(SDL_LOG_PRIORITY_INFO);
    print_debug_info();
  }

  window = SDL_CreateWindow(NAME,
			    SDL_WINDOWPOS_UNDEFINED,
			    SDL_WINDOWPOS_UNDEFINED,
			    WIDTH, HEIGHT,
			    SDL_WINDOW_OPENGL);
  if (window == NULL) {
    SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Could not create window: %s\n", SDL_GetError());
    return 1;
  }

  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

  SDL_GLContext gl_context = SDL_GL_CreateContext(window);
  if (gl_context == NULL) {
    SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Could not create OpenGL context: %s\n", SDL_GetError());
    SDL_DestroyWindow(window);
    return 1;
  }

  if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) {
    std::cout << "Failed to initialize GLAD" << std::endl;
    return -1;
  }

  SDL_Log("OpenGL version: %s\n", glGetString(GL_VERSION));

  int running = 1;
  while (running) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      switch (event.type) {
      case SDL_QUIT:
	running = 0;
	break;
      }
    }

    glClearColor(0.2f, 0.2f, 0.7f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    SDL_GL_SwapWindow(window);

    SDL_Delay(300);

    glClearColor(0.7f, 0.2f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    SDL_GL_SwapWindow(window);

    SDL_Delay(300);

    glClearColor(0.2f, 0.8f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    SDL_GL_SwapWindow(window);

    SDL_Delay(300);
  }

  SDL_GL_DeleteContext(gl_context);
  SDL_DestroyWindow(window);

  SDL_Quit();

  return 0;
}
