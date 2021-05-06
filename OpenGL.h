#pragma once

#define GL_GLEXT_PROTOTYPES 1

#include <GL/gl.h>

#include <string>
#include <vector>

class GlShader final
{
public:
  static GlShader Create(GLenum type, const std::string& source)
  {
    GlShader shader(glCreateShader(type));

    const char* source_ptr = source.c_str();

    GLint source_length = source.size();

    glShaderSource(shader.mID, 1, &source_ptr, &source_length);

    glCompileShader(shader.mID);

    return shader;
  }

  GlShader(GlShader&& other)
    : mID(other.mID)
  {
    other.mID = 0;
  }

  GlShader() = delete;

  ~GlShader()
  {
    if (mID != 0)
      glDeleteShader(mID);
  }

  GLuint ID() const { return mID; }

  auto GetCompileResult() const -> std::pair<bool, std::string>
  {
    GLint success = GL_TRUE;

    glGetShaderiv(mID, GL_COMPILE_STATUS, &success);

    if (success == GL_TRUE)
      return { true, "" };

    GLint log_size = 0;

    glGetShaderiv(mID, GL_INFO_LOG_LENGTH, &log_size);

    std::string info_log;

    GLsizei max_length = GLsizei(log_size);

    info_log.resize(max_length);

    glGetShaderInfoLog(mID, max_length, &max_length, &info_log[0]);

    info_log.resize(max_length);

    return { false, info_log };
  }

private:
  GlShader(GLuint id)
    : mID(id)
  {}

  GLuint mID = 0;
};

class GlProgram final
{
public:
  static GlProgram MakeInvalid() { return GlProgram(0); }

  static GlProgram Create(std::vector<GLuint> shaders)
  {
    GlProgram program(glCreateProgram());

    for (const auto& shader : shaders)
      glAttachShader(program.mID, shader);

    glLinkProgram(program.mID);

    glValidateProgram(program.mID);

    for (const auto& shader : shaders)
      glDetachShader(program.mID, shader);

    return program;
  }

  GlProgram() = delete;

  GlProgram(GlProgram&& other)
    : mID(other.mID)
  {
    other.mID = 0;
  }

  ~GlProgram()
  {
    if (mID != 0)
      glDeleteProgram(mID);
  }

  bool IsValid() const noexcept { return !!mID; }

  GLuint ID() { return mID; }

  auto GetLinkResult() const -> std::pair<bool, std::string>
  {
    GLint is_linked = 0;

    glGetProgramiv(mID, GL_LINK_STATUS, &is_linked);

    if (is_linked == GL_TRUE)
      return { true, "" };

    GLint log_length = 0;

    glGetProgramiv(mID, GL_INFO_LOG_LENGTH, &log_length);

    std::string log;

    log.resize(log_length);

    GLsizei max_length = GLsizei(log_length);

    glGetProgramInfoLog(mID, max_length, &max_length, &log[0]);

    log.resize(max_length);

    return { false, log };
  }

private:
  GlProgram(GLuint id)
    : mID(id)
  {}

  GLuint mID = 0;
};
