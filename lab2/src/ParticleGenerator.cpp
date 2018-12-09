#include "ParticleGenerator.h"

#include <glad/glad.h>
#include <cstdlib>

#include <iostream>

ParticleGenerator::ParticleGenerator(glm::vec3 pos)
  : pos(pos) {
}

SnowGenerator::SnowGenerator() : SnowGenerator(glm::vec3(0, 20, 0)) {

}

SnowGenerator::SnowGenerator(glm::vec3 pos)
  : ParticleGenerator(pos), snowflakes(200), size(20.f, 20.f) {
  for (Snowflake &s : snowflakes) {
    if (rand()/(float)RAND_MAX < 0.4) {
      s.reset(generate_position());
    }
  }
}

void SnowGenerator::setup_rendering() {
  glGenVertexArrays(1, &cloud_VAO);
  glGenBuffers(1, &cloud_VBO);
  glGenBuffers(1, &cloud_EBO);

  cloud_shader.load("cloud.vert", "cloud.frag");

  const GLfloat cube[] = {
			  // front           // color
			  -0.5, -0.5,  0.5,  0.5, 0.5, 0.5,
			  0.5, -0.5,  0.5,   0.8, 0.8, 0.8,
			  0.5,  0.5,  0.5,   0.6, 0.6, 0.6,
			  -0.5,  0.5,  0.5,  0.8, 0.85, 0.82,
			  // back
			  -0.5, -0.5, -0.5,  0.67, 0.72, 0.7,
			  0.5, -0.5, -0.5,   0.85, 0.85, 0.95,
			  0.5,  0.5, -0.5,   0.4, 0.4, 0.42,
			  -0.5,  0.5, -0.5,  0.95, 0.95, 0.91,
  };

  GLuint cube_elements[] = {
			      // front
			      0, 1, 2,
			      2, 3, 0,
			      // right
			      1, 5, 6,
			      6, 2, 1,
			      // back
			      7, 6, 5,
			      5, 4, 7,
			      // left
			      4, 0, 3,
			      3, 7, 4,
			      // bottom
			      4, 5, 1,
			      1, 0, 4,
			      // top
			      3, 2, 6,
			      6, 7, 3,
  };

  glBindVertexArray(cloud_VAO);
  glBindBuffer(GL_ARRAY_BUFFER, cloud_VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(cube), &cube[0], GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cloud_EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cube_elements), &cube_elements[0], GL_STATIC_DRAW);

  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (void*)0);
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1 , 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(float)));
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);

  // setup particle rendering
  glGenVertexArrays(1, &snow_VAO);
  glGenBuffers(1, &snow_VBO);
  glGenBuffers(1, &snow_EBO);

  snowflake_shader.load("snowflake.vert", "snowflake.frag");

  // snowflake
  glBindVertexArray(snow_VAO);
  glBindBuffer(GL_ARRAY_BUFFER, snow_VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(Snowflake::vertices[0])*20, Snowflake::vertices, GL_STATIC_DRAW);

  unsigned inds[] = {0, 1, 2, 2, 3, 0};
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, snow_EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned) * 6, &inds[0], GL_STATIC_DRAW);

  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (GLvoid*)(3 * sizeof(float)));
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
}

glm::vec3 SnowGenerator::generate_position() const {
  const float x = pos.x-size.x/2.f+rand()/(float)RAND_MAX*size.x;
  const float z = pos.z-size.y/2.f+rand()/(float)RAND_MAX*size.y;

  return glm::vec3(x, pos.y, z);
}

void SnowGenerator::tick(float delta) {
  for (Snowflake &s : snowflakes) {
    if (s.alive) {
      s.tick(delta);
    } else if (rand()/(double)RAND_MAX < 0.7) {
      s.reset(generate_position());
    }
  }

  pos += delta * speed;
}

void SnowGenerator::render(const Camera &camera) {
  const glm::mat4 view = camera.get_view();
  const glm::mat4 projection = camera.get_projection();

  // cloud cube
  const GLint obj_MVP = glGetUniformLocation(cloud_shader.id, "MVP");
  const GLint obj_MV = glGetUniformLocation(cloud_shader.id, "MV");

  const glm::mat4 model = glm::translate(pos) * glm::scale(glm::vec3(size.x, size.x/4.f, size.y));
  const glm::mat4 MVP = projection * view * model;

  cloud_shader.use();
  glUniformMatrix4fv(obj_MVP, 1, GL_FALSE, glm::value_ptr(MVP));
  glUniformMatrix4fv(obj_MV, 1, GL_FALSE, glm::value_ptr(MVP));
  glBindVertexArray(cloud_VAO);
  glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
  glBindVertexArray(0);

  // draw snowflakes
  const glm::vec3 orientation = glm::vec3(0.0f, 0.0f, 1.0f);
  const glm::vec3 rot_axis = glm::cross(orientation, camera.front);

  const float cos_phi = glm::dot(camera.front, orientation);
  const float angle = std::acos(cos_phi);
  const glm::mat4 rot = glm::rotate(angle, rot_axis); // documentation is wrong, it's radians, not degrees!!!

  const GLint snow_MVP = glGetUniformLocation(snowflake_shader.id, "MVP");
  const GLint snow_MV = glGetUniformLocation(snowflake_shader.id, "MV");

  snowflake_shader.use();
  glBindTexture(GL_TEXTURE_2D, Snowflake::texture);
  glBindVertexArray(snow_VAO);

  for (Snowflake &s : snowflakes) {
    if (!s.alive)
      continue;

    const glm::mat4 scale = glm::scale(s.scale*glm::vec3(2.f, 2.f, 2.f));
    const glm::mat4 translate = glm::translate(s.pos);
    const glm::mat4 model = translate * rot * scale;

    const glm::mat4 MVP =  projection * view * model;

    glUniformMatrix4fv(snow_MVP, 1, GL_FALSE, glm::value_ptr(MVP));
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
  }

  glBindVertexArray(0);
}
