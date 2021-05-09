#pragma once

#include <glm/glm.hpp>

class Camera final
{
public:
  glm::mat4 GetMVP(const glm::mat4& modelTx = glm::mat4(1.0f)) const noexcept;

  void SetAspectRatio(float aspect) noexcept;

private:
  float mRotX = 0;
  float mRotY = 0;
  float mAspect = 1.0f;
};
