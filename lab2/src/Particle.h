#ifndef PARTICLE_H
#define PARTICLE_H

#include <glm/glm.hpp>

#include "Camera.h"

class Particle {
 public:
  Particle() = default;
  Particle(glm::vec3 pos, glm::vec3 speed);

  virtual void tick(float delta);

  virtual void render(const Camera &camera) = 0;

 private:
  glm::vec3 pos;
  glm::vec3 speed;
};

class Snowflake : public Particle {
 public:
  virtual void render(const Camera &camera);

  static unsigned texture;

 private:
  // quad
  static const float *vertices;
};

#endif // PARTICLE_H
