#include "Shader.h"

#define INFO_LOG_SIZE 512

Shader::Shader(const char* vertexPath, const char* fragmentPath) {
  std::string vertexCode;
  std::string fragmentCode;
  std::ifstream vShaderFile;
  std::ifstream fShaderFile;

  vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
  fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

  try {
    vShaderFile.open(vertexPath);
    fShaderFile.open(fragmentPath);
    std::stringstream vShaderStream, fShaderStream;

    vShaderStream << vShaderFile.rdbuf();
    fShaderStream << fShaderFile.rdbuf();

    vShaderFile.close();
    fShaderFile.close();

    vertexCode = vShaderStream.str();
    fragmentCode = fShaderStream.str();
  } catch (std::ifstream::failure e) {
    std::cerr << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ" << std::endl;
  }

  const char *vShaderCode = vertexCode.c_str();
  const char *fShaderCode = fragmentCode.c_str();

  unsigned int vertex, fragment;
  vertex = compileShader(vShaderCode, GL_VERTEX_SHADER);
  fragment = compileShader(fShaderCode, GL_FRAGMENT_SHADER);

  linkProgram(vertex, fragment);
}

unsigned int Shader::compileShader(const char *source, GLenum shader_type) {
  unsigned shader = glCreateShader(shader_type);
  glShaderSource(shader, 1, &source, NULL);
  glCompileShader(shader);

  int success;
  char infoLog[INFO_LOG_SIZE];
  glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(shader, INFO_LOG_SIZE, NULL, infoLog);
    switch (shader_type) {
    case GL_VERTEX_SHADER:
      std::cerr << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
      break;
    case GL_FRAGMENT_SHADER:
      std::cerr << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
      break;
    default:
      std::cerr << "ERROR::UNKNOWN_SHADER_TYPE\n" << infoLog << std::endl;
      break;
    }
  }

  return shader;
}

void Shader::linkProgram(unsigned vertex, unsigned fragment) {
  id = glCreateProgram();
  glAttachShader(id, vertex);
  glAttachShader(id, fragment);

  glLinkProgram(id);

  int success;
  char infoLog[INFO_LOG_SIZE];
  glGetProgramiv(id, GL_LINK_STATUS, &success);
  if (!success) {
    glGetProgramInfoLog(id, INFO_LOG_SIZE, NULL, infoLog);
    std::cerr << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
  }

  // we don't need shaders anymore
  glDeleteShader(vertex);
  glDeleteShader(fragment);

  std::cout << "Successfully loaded shaders!" << std::endl;
}

void Shader::use() const {
  glUseProgram(id);
}

void Shader::setBool(const std::string &name, bool value) const{
  glUniform1i(glGetUniformLocation(id, name.c_str()), (int)value);
}
void Shader::setInt(const std::string &name, int value) const{
  glUniform1i(glGetUniformLocation(id, name.c_str()), value);
}
void Shader::setFloat(const std::string &name, float value) const{
  glUniform1f(glGetUniformLocation(id, name.c_str()), value);
}
