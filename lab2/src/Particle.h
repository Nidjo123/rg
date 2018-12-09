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

  virtual void reset(glm::vec3 pos);

  glm::vec3 pos;
  glm::vec3 speed;
  bool alive;
};

class Snowflake : public Particle {
 public:
  Snowflake();
  Snowflake(glm::vec3 pos);

  virtual void render(const Camera &camera) override;

  virtual void reset(glm::vec3 pos);

  static unsigned texture;

  // private:
  // quad
  static const float *vertices;

  float scale;
};

#endif // PARTICLE_H
