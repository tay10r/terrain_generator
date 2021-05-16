#pragma once

#include <stddef.h>

namespace terra {

class LineObserver
{
public:
  virtual ~LineObserver() = default;

  virtual void Observe(const float* heightAndRgbData) = 0;
};

} // namespace terra
