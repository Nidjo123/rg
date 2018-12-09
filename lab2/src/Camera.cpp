#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Camera.h"

void Camera::update_position(float delta, bool key_down[4]) {
  const float cameraSpeed = speed * delta;
  if (key_down[0]) // W
    pos += cameraSpeed * front;
  if (key_down[2]) // S
    pos -= cameraSpeed * front;
  if (key_down[1]) // A
    pos -= glm::normalize(glm::cross(front, up)) * cameraSpeed;
  if (key_down[3]) // D
    pos += glm::normalize(glm::cross(front, up)) * cameraSpeed;
}

void Camera::update_orientation(float delta, float xoffset, float yoffset) {
  const float sensitivity = 10.0f * delta;
  xoffset *= sensitivity;
  yoffset *= sensitivity;

  yaw += xoffset;
  pitch -= yoffset;

  if (pitch > 89.0f)
    pitch = 89.0f;
  if (pitch < -89.0f)
    pitch = -89.0f;
}

glm::mat4 Camera::get_view() {
  glm::vec3 camFront;
  camFront.x = std::cos(glm::radians(pitch)) * std::cos(glm::radians(yaw));
  camFront.y = std::sin(glm::radians(pitch));
  camFront.z = std::cos(glm::radians(pitch)) * std::sin(glm::radians(yaw));
  front = glm::normalize(camFront);

  view = glm::lookAt(pos, pos + front, up);

  return view;
}

glm::mat4 Camera::get_projection() const {
  return projection;
}
