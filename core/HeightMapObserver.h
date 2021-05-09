#pragma once

#include <stddef.h>

/// @brief Used for whenever the height map gets updated by the backend.
class HeightMapObserver
{
public:
  virtual ~HeightMapObserver() = default;

  virtual void Observe(const float* data, size_t w, size_t h) = 0;
};
