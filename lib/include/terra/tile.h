#pragma once

#include <array>

#include <stddef.h>

namespace terra {

constexpr size_t
TileSize() noexcept
{
  return 256;
}

class Tile final
{
public:
  /// The height and RGB color data, interleaved.
  using Buffer = std::array<float, TileSize() * TileSize() * 4>;

  Tile(size_t offsetX, size_t offsetY, size_t width, size_t height) noexcept
    : mOffsetX(offsetX)
    , mOffsetY(offsetY)
    , mWidth(width)
    , mHeight(height)
  {}

  const float* GetLinePtr(size_t line) const noexcept
  {
    return mBuffer.data() + (line * mWidth * 4);
  }

  const Buffer& GetBuffer() const noexcept { return mBuffer; }

  Buffer& GetBuffer() noexcept { return mBuffer; }

  size_t GetOffsetX() const noexcept { return mOffsetX; }

  size_t GetOffsetY() const noexcept { return mOffsetY; }

  size_t GetWidth() const noexcept { return mWidth; }

  size_t GetHeight() const noexcept { return mHeight; }

  float GetHeightAt(size_t x, size_t y) const noexcept;

  /// @return True on success, false if @p bufferSize is too small or too large.
  bool ToPositionBuffer(float* buffer, size_t bufferSize) const noexcept;

  /// @return True on success, false if @p bufferSize is too small or too large.
  bool ToNormalBuffer(float* buffer, size_t bufferSize) const noexcept;

private:
  Buffer mBuffer;

  size_t mOffsetX;

  size_t mOffsetY;

  size_t mWidth;

  size_t mHeight;
};

} // namespace terra
