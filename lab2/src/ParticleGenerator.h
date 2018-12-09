#ifndef PARTICLE_GENERATOR_H
#define PARTICLE_GENERATOR_H

#include <vector>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Camera.h"
#include "Particle.h"
#include "Shader.h"

class ParticleGenerator {
 public:
  ParticleGenerator() = default;
  ParticleGenerator(glm::vec3 pos);

  virtual void tick(float delta) = 0;
  virtual void render(const Camera &camera) = 0;

 protected:
  glm::vec3 pos;
};

class SnowGenerator : public ParticleGenerator {
 public:
  SnowGenerator();
  SnowGenerator(glm::vec3 pos);

  virtual void tick(float delta) override;
  virtual void render(const Camera &camera) override;

  void setup_rendering();

 private:
  glm::vec3 generate_position() const;

  unsigned int snow_VBO;
  unsigned int snow_VAO;
  unsigned int snow_EBO;

  Shader snowflake_shader;

  std::vector<Snowflake> snowflakes;
  int n_particles = 1000;

  glm::vec2 size;
};

#endif // PARTICLE_GENERATOR_H
