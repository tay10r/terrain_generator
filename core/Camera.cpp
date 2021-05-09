#include "core/Camera.h"

#include <glm/gtx/transform.hpp>

glm::mat4
Camera::GetMVP(const glm::mat4& modelTx) const noexcept
{
  auto proj = glm::perspective(glm::radians(45.0f), mAspect, 0.1f, 100.0f);

  auto eye = glm::vec3(3, 3, 3);

  // TODO : compute eye from rotation angles.

  auto center = glm::vec3(0, 0, 0);

  auto up = glm::vec3(0, 1, 0);

  auto view = glm::lookAt(eye, center, up);

  return modelTx * view * proj;
}

void
Camera::SetAspectRatio(float aspect) noexcept
{
  mAspect = aspect;
}
