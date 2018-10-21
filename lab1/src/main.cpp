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

unsigned int object_VBO, bspline_VBO, tangent_VBO;
unsigned int object_VAO, bspline_VAO, tangent_VAO;
unsigned int object_EBO;

Shader shader;
Shader bspline_shader;

glm::mat4 view = glm::lookAt(glm::vec3(-7.f, 15.f, -5.f),
			     glm::vec3(5.0f, 5.0f, 30.0f),
			     glm::vec3(0.0f, 1.0f, 0.0f));
glm::mat4 projection = glm::perspective(glm::radians(45.0f), 800.f / 600.f, 0.1f, 100.0f);
glm::mat4 model;
glm::mat4 MVP;
glm::mat4 MV;

const int samples_per_segment = 50;
BSpline bspline("points");
std::vector<float> bspline_samples(bspline.segments() * samples_per_segment * 3);
std::vector<float> bspline_tangents(bspline.segments() * samples_per_segment * 3);

std::vector<float> tangent(6); // two 3D points

Object obj;
std::vector<unsigned> indices;
std::vector<float> obj_data;

const int duration = 20; // ticks

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
  glGenVertexArrays(1, &tangent_VAO);

  glGenBuffers(1, &object_VBO);
  glGenBuffers(1, &object_EBO);
  glGenBuffers(1, &bspline_VBO);
  glGenBuffers(1, &tangent_VBO);

  shader.load("shader.vert", "shader.frag");
  bspline_shader.load("bspline.vert", "bspline.frag");

  int i = 0;
  for (int seg = 0; seg < bspline.segments(); seg++) {
    const float delta = 1.0f / samples_per_segment;
    float t = 0.0f;
    for (int samp = 0; samp < samples_per_segment; samp++) {
      glm::vec3 value = bspline.value(seg, t);
      bspline_samples[i] = value[0];
      bspline_samples[i+1] = value[1];
      bspline_samples[i+2] = value[2];
      glm::vec3 tangent = bspline.tangent(seg, t);
      bspline_tangents[i] = tangent[0];
      bspline_tangents[i+1] = tangent[1];
      bspline_tangents[i+2] = tangent[2];

      i += 3;
      t += delta;
    }
  }

  float cols[] = {1.f, 0.f, 0.f,
		  0.f, 1.f, 0.f,
		  0.f, 0.f, 1.f,
		  1.f, 1.f, 0.f,
		  0.f, 1.f, 1.f,
		  1.f, 0.f, 1.f,
		  .5f, .5f, .5f,
  };

  const size_t ncols = sizeof(cols)/sizeof(cols[0]);

  for (int i = 0; i < obj.attrib.vertices.size() / 3; i++) {
    obj_data.push_back(obj.attrib.vertices[i*3]);
    obj_data.push_back(obj.attrib.vertices[i*3+1]);
    obj_data.push_back(obj.attrib.vertices[i*3+2]);

    obj_data.push_back(obj.attrib.normals[i*3]);
    obj_data.push_back(obj.attrib.normals[i*3+1]);
    obj_data.push_back(obj.attrib.normals[i*3+2]);

    obj_data.push_back(cols[i%ncols]);
    obj_data.push_back(cols[i%ncols+1]);
    obj_data.push_back(cols[i%ncols+2]);
  }

  for (const auto& shape : obj.shapes) {
    for (const auto& i : shape.mesh.indices) {
      indices.push_back(i.vertex_index);
    }
  }

  glBindVertexArray(object_VAO);
  glBindBuffer(GL_ARRAY_BUFFER, object_VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(obj_data[0])*obj_data.size(), &obj_data[0], GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, object_EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned) * indices.size(), &indices[0], GL_STATIC_DRAW);

  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (GLvoid*)(3 * sizeof(float)));
  glEnableVertexAttribArray(2);
  glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (GLvoid*)(6 * sizeof(float)));
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);

  // setup BSpline VAO
  glBindVertexArray(bspline_VAO);
  glBindBuffer(GL_ARRAY_BUFFER, bspline_VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(float) * bspline_samples.size(), &bspline_samples[0], GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);

  // setup tangent VAO
  glBindVertexArray(tangent_VAO);
  glBindBuffer(GL_ARRAY_BUFFER, tangent_VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(float) * tangent.size(), &tangent[0], GL_DYNAMIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
}

void render() {
  glClearColor(0.3f, 0.3f, 0.7f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  const GLint obj_MVP = glGetUniformLocation(shader.id, "MVP");
  const GLint obj_MV = glGetUniformLocation(shader.id, "MV");
  const GLint bspline_MVP = glGetUniformLocation(bspline_shader.id, "MVP");
  const GLint bspline_col = glGetUniformLocation(bspline_shader.id, "color");

  shader.use();
  glUniformMatrix4fv(obj_MVP, 1, GL_FALSE, glm::value_ptr(MVP));
  glUniformMatrix4fv(obj_MV, 1, GL_FALSE, glm::value_ptr(MV));
  glBindVertexArray(object_VAO);
  glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
  glBindVertexArray(0);

  MVP = projection * view; // no model transform
  bspline_shader.use();
  glBindVertexArray(bspline_VAO);
  glUniformMatrix4fv(bspline_MVP, 1, GL_FALSE, glm::value_ptr(MVP));
  glUniform3f(bspline_col, 0.0f, 1.0f, 0.0f);
  glDrawArrays(GL_LINE_STRIP, 0, bspline_samples.size() / 3);
  glBindVertexArray(0);

  // draw tangent
  bspline_shader.use();
  glBindVertexArray(tangent_VAO);
  glUniformMatrix4fv(bspline_MVP, 1, GL_FALSE, glm::value_ptr(MVP));
  glUniform3f(bspline_col, 1.0f, 0.0f, 0.0f);
  glBindBuffer(GL_ARRAY_BUFFER, tangent_VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(float) * tangent.size(), &tangent[0], GL_DYNAMIC_DRAW);
  glDrawArrays(GL_LINES, 0, tangent.size() / 3);
  glBindVertexArray(0);

  GLenum err;
  while((err = glGetError()) != GL_NO_ERROR) {
    printf("OpenGL Error: %x\n", err);
  }
}

void tick() {
  // calculate object position on the spline
  const int curr_pt = SDL_GetTicks() / duration % (bspline_samples.size()/3);
  const float dx = bspline_samples[curr_pt*3];
  const float dy = bspline_samples[curr_pt*3+1];
  const float dz = bspline_samples[curr_pt*3+2];

  const glm::vec3 tang(bspline_tangents[curr_pt*3],
		       bspline_tangents[curr_pt*3+1],
		       bspline_tangents[curr_pt*3+2]);
  const float scale = 10.0f;
  tangent[0] = dx;
  tangent[1] = dy;
  tangent[2] = dz;
  tangent[3] = dx + scale*tang[0];
  tangent[4] = dy + scale*tang[1];
  tangent[5] = dz + scale*tang[2];

  const glm::vec3 orientation = glm::vec3(0.0f, 0.0f, 1.0f);
  const glm::vec3 rot_axis = glm::cross(orientation, tang);
  const float cos_phi = glm::dot(tang, orientation);
  const float angle = std::acos(cos_phi);// / M_PI * 180.0f;
  const glm::mat4 rot = glm::rotate(angle, rot_axis); // documentation is wrong, it's radians, not degrees!!!
  const glm::mat4 scale_ = glm::scale(glm::vec3(10.f, 10.f, 10.f));
  const glm::mat4 translate = glm::translate(glm::vec3(dx, dy, dz));

  model = translate * rot * scale_;

  MVP = projection * view * model;
  MV = view * model;
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

  init();

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

    render();

    SDL_GL_SwapWindow(window);

    SDL_Delay(1);
  }

  SDL_GL_DeleteContext(gl_context);
  SDL_DestroyWindow(window);

  SDL_Quit();

  return 0;
}
