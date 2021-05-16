#pragma once

#include <terra/line_observer.h>

#include <memory>

#include <stddef.h>

namespace terra {

class PngWriter : public LineObserver
{
public:
  /// @param width The width of the terrain.
  ///
  /// @param height The height of the terrain.
  ///
  /// @param heightPath The path to save the height file at.
  ///
  /// @param colorPath The path to save the color file at.
  static auto Make(size_t width,
                   size_t height,
                   const char* heightPath,
                   const char* colorPath) -> std::unique_ptr<PngWriter>;

  virtual ~PngWriter() = default;

  virtual void SetHeightRange(float min, float max) = 0;
};

} // namespace terra
