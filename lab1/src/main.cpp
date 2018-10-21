#include <glad/glad.h>

#define SDL_MAIN_HANDLED
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

#include "Object.h"
#include "Shader.h"
#include "BSpline.h"

#define DEBUG 1

static const char *NAME = "RG -- B-Spline";
static const int WIDTH = 800;
static const int HEIGHT = 600;

unsigned int object_VBO, bspline_VBO;
unsigned int object_VAO, bspline_VAO;
unsigned int object_EBO;

Shader shader;
Shader bspline_shader;

glm::mat4 view = glm::lookAt(glm::vec3(0.f, 5.f, -15.f),
			     glm::vec3(5.0f, 5.0f, 30.0f),
			     glm::vec3(0.0f, 1.0f, 0.0f));
glm::mat4 projection = glm::perspective(glm::radians(45.0f), 800.f / 600.f, 0.1f, 100.0f);
glm::mat4 model;
glm::mat4 MVP;

const int samples_per_segment = 20;
BSpline bspline("points");
std::vector<float> bspline_samples(bspline.segments() * samples_per_segment * 3);

Object obj;
std::vector<unsigned> indices;

const int duration = 10; // ticks

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

void init() {
  glViewport(0, 0, WIDTH, HEIGHT);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);

  //glCullFace(GL_FRONT);

  glGenVertexArrays(1, &object_VAO);
  glGenVertexArrays(1, &bspline_VAO);

  glGenBuffers(1, &object_VBO);
  glGenBuffers(1, &object_EBO);
  glGenBuffers(1, &bspline_VBO);

  shader.load("shader.vert", "shader.frag");
  bspline_shader.load("bspline.vert", "bspline.frag");

  int i = 0;
  for (int seg = 0; seg < bspline.segments(); seg++) {
    const float delta = 1.0f / samples_per_segment;
    float t = 0.0f;
    for (int samp = 0; samp < samples_per_segment; samp++) {
      glm::vec3 val = bspline.value(seg, t);
      bspline_samples[i] = val[0];
      bspline_samples[i+1] = val[1];
      bspline_samples[i+2] = val[2];
      i += 3;
      t += delta;
    }
  }
}

void render() {
  glClearColor(0.3f, 0.3f, 0.7f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // render BSpline by sampling over every segment
  const GLint locationID = glGetUniformLocation(bspline_shader.id, "MVP");
  const GLint bspline_MVP = glGetUniformLocation(shader.id, "MVP");

  shader.use();
  glUniformMatrix4fv(locationID, 1, GL_FALSE, glm::value_ptr(MVP));
  glBindVertexArray(object_VAO);
  glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
  glBindVertexArray(0);

  MVP = projection * view; // no model transform
  bspline_shader.use();
  glBindVertexArray(bspline_VAO);
  glUniformMatrix4fv(bspline_MVP, 1, GL_FALSE, glm::value_ptr(MVP));
  glDrawArrays(GL_LINE_STRIP, 0, bspline_samples.size() / 3);
  glBindVertexArray(0);
}

void tick() {
  // calculate object position on the spline
  const int curr_pt = SDL_GetTicks() / duration % (bspline_samples.size()/3);
  std::cout << curr_pt << std::endl;
  float dx = bspline_samples[curr_pt*3];
  float dy = bspline_samples[curr_pt*3+1];
  float dz = bspline_samples[curr_pt*3+2];

  model = glm::translate(glm::vec3(dx, dy, dz));

  MVP = projection * view * model;
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    std::cerr << "Must pass path to obj as argument!" << std::endl;
    exit(1);
  }

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
  int nrAttributes;
  glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &nrAttributes);
  SDL_Log("Maximum nr of vertex attributes supported: %d\n", nrAttributes);

  obj.load(argv[1]);

  for (const auto& shape : obj.shapes) {
    for (const auto& i : shape.mesh.indices) {
      indices.push_back(i.vertex_index);
    }
    std::cout << std::endl;
  }

  init();

  glBindVertexArray(object_VAO);
  glBindBuffer(GL_ARRAY_BUFFER, object_VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(obj.attrib.vertices[0])*obj.attrib.vertices.size(), &obj.attrib.vertices[0], GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, object_EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned) * indices.size(), &indices[0], GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);
  glBindVertexArray(0);

  // setup BSpline VAO
  glBindVertexArray(bspline_VAO);
  glBindBuffer(GL_ARRAY_BUFFER, bspline_VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(float) * bspline_samples.size(), &bspline_samples[0], GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);
  glBindVertexArray(0);

  int cnt = 0;
  for (auto x : bspline_samples) {
    std::cout << x << " ";
    if ((++cnt) % 3 == 0) std::cout << std::endl;
  }

  int running = 1;
  while (running) {
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
      }
    }

    tick();

    const unsigned period = 300000;
    unsigned ticks = SDL_GetTicks() % period;

    //MVP = projection * view * glm::rotate(ticks/(float)period*360.f, glm::vec3(0.0f, 1.0f, 0.0f));

    GLenum err;
    while((err = glGetError()) != GL_NO_ERROR) {
      printf("OpenGL Error: %x\n", err);
    }

    render();

    SDL_GL_SwapWindow(window);

    SDL_Delay(1);
  }

  SDL_GL_DeleteContext(gl_context);
  SDL_DestroyWindow(window);

  SDL_Quit();

  return 0;
}
