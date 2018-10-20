#ifndef SHADER_H
#define SHADER_H

// mainly from https://learnopengl.com/Getting-started/Shaders

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

#include <glad/glad.h>

class Shader {
public:
  Shader(const char* vertexPath, const char* fragmentPath);

  void use() const;

  void setBool(const std::string &name, bool value) const;
  void setInt(const std::string &name, int value) const;
  void setFloat(const std::string &name, float value) const;

private:
  unsigned int compileShader(const char *source, GLenum shader_type);
  void linkProgram(unsigned vertex, unsigned fragment);

  unsigned int id;
};

#endif
