#include "BSpline.h"

#include <fstream>

BSpline::BSpline(const char *path) {
  std::ifstream file(path);

  float x, y, z;
  while (!file.eof()) {
    file >> x >> y >> z;
    cpoints.push_back(glm::vec3(x, y, z));
  }

  file.close();

  assert(cpoints.size() >= 4);
}

int BSpline::points() const {
  return cpoints.size();
}

int BSpline::segments() const {
  return points() - 4 + 1;
}

glm::mat3x4 BSpline::makeR(int segment) const {
  glm::mat3x4 R;

  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 4; j++) {
      R[i][j] = cpoints[segment + j][i];
    }
  }

  return R;
}

glm::vec3 BSpline::value(int segment, float t) const {
  const glm::vec4 T3 = glm::vec4(t*t*t, t*t, t, 1.0f);
  glm::mat4 B3;
  // column-major ordering!
  B3[0][0] = -1.0f;
  B3[0][1] = 3.0f;
  B3[0][2] = -3.0f;
  B3[0][3] = 1.0f;
  B3[1][0] = 3.0f;
  B3[1][1] = -6.0f;
  B3[1][2] = 0.0f;
  B3[1][3] = 4.0f;
  B3[2][0] = -3.0f;
  B3[2][1] = 3.0f;
  B3[2][2] = 3.0f;
  B3[2][3] = 1.0f;
  B3[3][0] = 1.0f;
  B3[3][1] = 0.0f;
  B3[3][2] = 0.0f;
  B3[3][3] = 0.0f;

  const glm::mat3x4 R = makeR(segment);

  return T3 / 6.0f * B3 * R;
}

glm::vec3 BSpline::tangent(int segment, float t) const {
  const glm::vec3 T2 = glm::vec3(t*t, t, 1.0f);
  glm::mat4x3 B3;
  // column-major ordering!
  B3[0][0] = -1.0f;
  B3[0][1] = 2.0f;
  B3[0][2] = -1.0f;
  B3[1][0] = 3.0f;
  B3[1][1] = -4.0f;
  B3[1][2] = 0.0f;
  B3[2][0] = -3.0f;
  B3[2][1] = 2.0f;
  B3[2][2] = 1.0f;
  B3[3][0] = 1.0f;
  B3[3][1] = 0.0f;
  B3[3][2] = 0.0f;

  const glm::mat3x4 R = makeR(segment);

  return glm::normalize(T2 / 2.0f * B3 * R);
}
