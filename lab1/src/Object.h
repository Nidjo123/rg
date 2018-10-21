#ifndef OBJECT_H
#define OBJECT_H

#include <string>
#include "tiny_obj_loader.h"

class Object {
public:
  Object() = default;
  Object(const char *path, const char *base_path = nullptr);

  void load(const char *path, const char *base_path = nullptr);

  void printInfo() const;

  tinyobj::attrib_t attrib;
  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;

private:
  void calculateVertexNormals();

  std::string path;
};

#endif
