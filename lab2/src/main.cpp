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

#include "stb_image.h"

#include "Shader.h"
#include "Camera.h"
#include "Object.h"
#include "Particle.h"

#define DEBUG 1

static const char *NAME = "RG -- Particles";
static const int WIDTH = 800;
static const int HEIGHT = 600;

bool key_down[4];
bool mouse_captured;
float xoffset;
float yoffset;

Camera camera;

unsigned int object_VBO;
unsigned int object_VAO;
unsigned int object_EBO;

unsigned int snow_VBO;
unsigned int snow_VAO;
unsigned int snow_EBO;

Shader shader;
Shader snowflake_shader;

glm::mat4 model;
glm::mat4 MVP;
glm::mat4 MV;

Object obj;
std::vector<unsigned> indices;
std::vector<float> obj_data;

unsigned int texture;

Snowflake snowflake;

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

  glGenVertexArrays(1, &object_VAO);
  glGenBuffers(1, &object_VBO);
  glGenBuffers(1, &object_EBO);

  glGenVertexArrays(1, &snow_VAO);
  glGenBuffers(1, &snow_VBO);
  glGenBuffers(1, &snow_EBO);

  shader.load("shader.vert", "shader.frag");
  snowflake_shader.load("snowflake.vert", "snowflake.frag");

  // load snowflake texture
  generate_texture("snow.bmp", &texture);
  Snowflake::texture = texture;

  // camera
  camera.view = glm::lookAt(glm::vec3(-7.f, 15.f, -10.f),
			    glm::vec3(0.0f, 0.0f, 0.0f),
			    glm::vec3(0.0f, 1.0f, 0.0f));
  camera.projection = glm::perspective(glm::radians(45.0f), 800.f / 600.f, 0.1f, 100.0f);

  camera.pos   = glm::vec3(-7.0f, 15.0f,  -10.0f);
  camera.front = glm::vec3(5.0f, 5.0f, 30.0f);
  camera.up    = glm::vec3(0.0f, 1.0f,  0.0f);
  camera.yaw   = 65.0f;
  camera.pitch = -10.0f;
  camera.speed = 15.0f;

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

  // snowflake
  glBindVertexArray(snow_VAO);
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
}

void render() {
  glClearColor(0.3f, 0.3f, 0.7f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  const GLint obj_MVP = glGetUniformLocation(shader.id, "MVP");
  const GLint obj_MV = glGetUniformLocation(shader.id, "MV");

  shader.use();
  glUniformMatrix4fv(obj_MVP, 1, GL_FALSE, glm::value_ptr(MVP));
  glUniformMatrix4fv(obj_MV, 1, GL_FALSE, glm::value_ptr(MV));
  glBindVertexArray(object_VAO);
  glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
  glBindVertexArray(0);

  snowflake.render(camera);

  // check for OpenGL errors
  GLenum err;
  while((err = glGetError()) != GL_NO_ERROR) {
    printf("OpenGL Error: %x\n", err);
  }
}

void cleanup() {
}

void tick(float t_delta) {
  /*
  const glm::vec3 orientation = glm::vec3(0.0f, 0.0f, 1.0f);
  const glm::vec3 rot_axis = glm::cross(orientation, tang);
  const float cos_phi = glm::dot(tang, orientation);
  const float angle = std::acos(cos_phi);// / M_PI * 180.0f;
  const glm::mat4 rot = glm::rotate(angle, rot_axis); // documentation is wrong, it's radians, not degrees!!!
  */
  const glm::mat4 scale_ = glm::scale(glm::vec3(10.f, 10.f, 10.f));
  //const glm::mat4 translate = glm::translate(glm::vec3(dx, dy, dz));

  model = scale_; //translate * rot * scale_;

  // process keyboard
  camera.update_position(t_delta, key_down);

  if (!mouse_captured) {
    xoffset = 0.0f;
    yoffset = 0.0f;
  }

  camera.update_orientation(t_delta, xoffset, yoffset);

  const glm::mat4 view = camera.get_view();
  const glm::mat4 projection = camera.get_projection();

  MVP =  projection * view * model;
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

  obj.load(argv[1]);

  init();

  unsigned last_ticks = 0u;

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
	xoffset = event.motion.xrel;
	yoffset = event.motion.yrel;
	break;
      }
    }

    const unsigned curr_ticks = SDL_GetTicks();
    const unsigned ticks_passed = curr_ticks - last_ticks;
    last_ticks = curr_ticks;
    const float t_delta = ticks_passed / 1000.0f;

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
