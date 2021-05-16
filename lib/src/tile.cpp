#include <terra/tile.h>

namespace terra {

float
Tile::GetHeightAt(size_t x, size_t y) const noexcept
{
  return mBuffer[((y * mWidth) + x) * 4];
}

bool
Tile::ToPositionBuffer(float* buffer, size_t bufferSize) const noexcept
{
  if (bufferSize != (mWidth * mHeight))
    return false;

  for (size_t i = 0; i < (mWidth * mHeight); i++) {

    size_t x = i % mWidth;
    size_t y = i / mHeight;

    float u = (x + 0.5f) / mWidth;
    float v = (y + 0.5f) / mHeight;

    buffer[(i * 3) + 0] = u;
    buffer[(i * 3) + 1] = v;
    buffer[(i * 3) + 2] = GetHeightAt(x, y);
  }

  return true;
}

bool
Tile::ToNormalBuffer(float* buffer, size_t bufferSize) const noexcept
{
  if (bufferSize != ((mWidth - 1) * (mHeight - 1)))
    return false;

  auto dx = 1.0f / mWidth;
  auto dy = 1.0f / mHeight;

  for (size_t y = 0; y < (mHeight - 1); y++) {

    for (size_t x = 0; x < (mWidth - 1); x++) {

      size_t i = ((y * mWidth) + x) * 3;

      auto h1 = GetHeightAt(x + 0, y + 0);
      auto h2 = GetHeightAt(x + 1, y + 0);
      auto h3 = GetHeightAt(x + 0, y + 1);

      // This is a simplification of the cross product,
      // since delta x and delta y are constants.
      buffer[i + 0] = dy * (h2 - h1);
      buffer[i + 1] = dx * (h3 - h1);
      buffer[i + 2] = -(dx * dy);
    }
  }

  return true;
}

} // namespace terra
