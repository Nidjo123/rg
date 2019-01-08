#include <glad/glad.h>

//#define SDL_MAIN_HANDLED
#include <SDL.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <string>
#include <vector>

#include "stb_image.h"

#include "Shader.h"

#define DEBUG 1

static const char *NAME = "RG -- RayMarching";
static const int WIDTH = 800;
static const int HEIGHT = 600;

bool key_down[4];
bool mouse_captured;
float xoffset;
float yoffset;

unsigned int quad_VBO;
unsigned int quad_VAO;

Shader shader;

glm::mat4 MVP;
glm::vec2 iResolution(WIDTH, HEIGHT);

float iTime;

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

void generate_texture(const char * const path, unsigned *tex) {
  int tex_width, tex_height, tex_nchannels;
  unsigned char *tex_data;

  glGenTextures(1, tex);
  glBindTexture(GL_TEXTURE_2D, *tex);

  tex_data = stbi_load("snow.bmp", &tex_width, &tex_height, &tex_nchannels, 0);

  if (tex_data) {
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, tex_width, tex_height, 0, GL_RGB, GL_UNSIGNED_BYTE, tex_data);
    glGenerateMipmap(GL_TEXTURE_2D);
  } else {
    std::cout << "Couldn't load texture " << path << std::endl;
  }

  stbi_image_free(tex_data);
}

void init() {
  glViewport(0, 0, WIDTH, HEIGHT);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);

  //glCullFace(GL_FRONT);

  // texture wrapping
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);

  // texture filtering
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  glGenVertexArrays(1, &quad_VAO);
  glGenBuffers(1, &quad_VBO);

  shader.load("shader.vert", "shader.frag");

  float quad_vertices[] = {
		  -1.f, 1.f,
		  1.f, 1.f,
		  -1.f, -1.f,
		  1.f, -1.f,
  };

  const size_t n_vertices = sizeof(quad_vertices)/sizeof(quad_vertices[0]);

  glBindVertexArray(quad_VAO);
  glBindBuffer(GL_ARRAY_BUFFER, quad_VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(quad_vertices[0])*n_vertices, quad_vertices, GL_STATIC_DRAW);

  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);

  // setup MVP
  const glm::mat4 projection = glm::ortho(0.0, 1.0, 1.0, 0.0);

  MVP =  projection;
}

void render() {
  glClearColor(0.f, 0.f, 0.f, 1.f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  const GLint MVP_location = glGetUniformLocation(shader.id, "MVP");
  const GLint iRes_location = glGetUniformLocation(shader.id, "iResolution");
  const GLint iTime_location = glGetUniformLocation(shader.id, "iTime");

  shader.use();

  // send uniforms
  glUniformMatrix4fv(MVP_location, 1, GL_FALSE, glm::value_ptr(MVP));
  glUniform2fv(iRes_location, 1, glm::value_ptr(iResolution));
  glUniform1f(iTime_location, iTime);

  glBindVertexArray(quad_VAO);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
  glBindVertexArray(0);

  // check for OpenGL errors
  GLenum err;
  while((err = glGetError()) != GL_NO_ERROR) {
    printf("OpenGL Error: %x\n", err);
  }
}

void cleanup() {
}

void tick(float t_delta) {
  // process keyboard
}

int main(int argc, char *argv[]) {
  SDL_Window *window = NULL;

  SDL_SetMainReady();
  SDL_Init(SDL_INIT_VIDEO  | SDL_INIT_AUDIO);

  if (DEBUG) {
    SDL_LogSetAllPriority(SDL_LOG_PRIORITY_INFO);
    print_debug_info();
  }

  SDL_CaptureMouse(SDL_TRUE);

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
  int nrAttributes;
  glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &nrAttributes);
  SDL_Log("Maximum nr of vertex attributes supported: %d\n", nrAttributes);

  init();

  unsigned last_ticks = 0u;

  int running = 1;
  while (running) {
    bool was_motion = false;
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      switch (event.type) {
      case SDL_QUIT:
	running = 0;
	break;
      case SDL_WINDOWEVENT_RESIZED:
      case SDL_WINDOWEVENT_SIZE_CHANGED:
	glViewport(0, 0, event.window.data1, event.window.data2);
	break;
      case SDL_KEYDOWN:
	if (event.key.keysym.sym == SDLK_w)
	  key_down[0] = true;
	if (event.key.keysym.sym == SDLK_s)
	  key_down[2] = true;
	if (event.key.keysym.sym == SDLK_a)
	  key_down[1] = true;
	if (event.key.keysym.sym == SDLK_d)
	  key_down[3] = true;
	if (event.key.keysym.sym == SDLK_c)
	  mouse_captured = !mouse_captured;
	if (event.key.keysym.sym == SDLK_ESCAPE)
	  SDL_CaptureMouse(SDL_FALSE);
	break;
      case SDL_KEYUP:
	if (event.key.keysym.sym == SDLK_w)
	  key_down[0] = false;
	if (event.key.keysym.sym == SDLK_s)
	  key_down[2] = false;
	if (event.key.keysym.sym == SDLK_a)
	  key_down[1] = false;
	if (event.key.keysym.sym == SDLK_d)
	  key_down[3] = false;
	break;
      case SDL_MOUSEMOTION:
	was_motion = true;
	xoffset = event.motion.xrel;
	yoffset = event.motion.yrel;
	break;
      }
    }

    if (!was_motion) {
      xoffset = 0;
      yoffset = 0;
    }

    if (mouse_captured) {
      SDL_SetRelativeMouseMode(SDL_TRUE);
    } else {
      SDL_SetRelativeMouseMode(SDL_FALSE);
    }

    const unsigned curr_ticks = SDL_GetTicks();
    const unsigned ticks_passed = curr_ticks - last_ticks;
    last_ticks = curr_ticks;
    const float t_delta = ticks_passed / 1000.0f;

    iTime = curr_ticks / 1000.0f;

    tick(t_delta);

    render();

    SDL_GL_SwapWindow(window);

    SDL_Delay(1);
  }

  cleanup();

  SDL_GL_DeleteContext(gl_context);
  SDL_DestroyWindow(window);

  SDL_Quit();

  return 0;
}
