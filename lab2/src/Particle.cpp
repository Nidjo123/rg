#include "Particle.h"
#include <iostream>

Particle::Particle(glm::vec3 pos, glm::vec3 speed)
  : pos(pos), speed(speed) {

}

void Particle::tick(float delta) {
  pos += delta * speed;
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
  std::cout << "Render!" << std::endl;
}
