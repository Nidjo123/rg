#ifndef BSPLINE_H
#define BSPLINE_H

#include <glm/glm.hpp>
#include <vector>

class BSpline {
public:
  BSpline(const char *path);

  int points() const;
  int segments() const;

  glm::vec3 value(int segment, float t) const;
  glm::vec3 tangent(int segment, float t) const;

private:
  glm::mat3x4 makeR(int segment) const;

  std::vector<glm::vec3> cpoints;
};

#endif
