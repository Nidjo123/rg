#ifndef CAMERA_H
#define CAMERA_H

#include <glm/glm.hpp>

class Camera {
public:
  Camera() = default;

  void update_position(float delta, bool keys[4]);
  void update_orientation(float delta, float xoffset, float yoffset);
  glm::mat4 get_view() const;
  glm::mat4 get_projection() const;

  glm::vec3 pos;
  glm::vec3 front;
  glm::vec3 up;

  float yaw;
  float pitch;
  float speed;

  glm::mat4 view;
  glm::mat4 projection;

private:
  void update_view();
};

#endif // CAMERA_H
