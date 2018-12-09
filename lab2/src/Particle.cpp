#include "Particle.h"
#include <iostream>

Particle::Particle(glm::vec3 pos, glm::vec3 speed)
  : pos(pos), speed(speed) {

}

void Particle::tick(float delta) {
  pos += delta * speed;

  if (pos.y < 0)
    alive = false;
}

void Particle::reset(glm::vec3 pos) {
  this->pos = pos;
  alive = true;
}

Snowflake::Snowflake() : Snowflake(glm::vec3(0, 15, 0)) {
}

Snowflake::Snowflake(glm::vec3 pos) : Particle(pos, glm::vec3(0, -2, 0)) {

}

void Snowflake::reset(glm::vec3 pos) {
  this->pos = pos;
  this->speed = (0.5f + rand()/(float)RAND_MAX) * glm::vec3(0, -5, 0);
  scale = (0.5f + rand()/(float)RAND_MAX);
  alive = true;
}

static const float quad[] = {// positions         // texture coords
			     0.5f,  0.5f, 0.0f,   1.0f, 1.0f,   // top right
			     0.5f, -0.5f, 0.0f,   1.0f, 0.0f,   // bottom right
			     -0.5f, -0.5f, 0.0f,   0.0f, 0.0f,   // bottom left
			     -0.5f,  0.5f, 0.0f,   0.0f, 1.0f    // top left
};
const float *Snowflake::vertices = quad;

unsigned Snowflake::texture;

void Snowflake::render(const Camera &camera) {
}
