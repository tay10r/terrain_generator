#include <terra/png_writer.h>

#include <terra/tile.h>

#include <algorithm>
#include <fstream>
#include <vector>

#include <png.h>

#include <zlib.h>

#include <iostream>

namespace terra {

namespace {

enum class PngKind
{
  Height,
  Color
};

class PngRowStream final
{
public:
  PngRowStream(const char* path, size_t w, size_t h, PngKind kind)
    : mFile(fopen(path, "wb"))
  {
    if (!mFile)
      return;

    const char* ver = PNG_LIBPNG_VER_STRING;

    mPng = png_create_write_struct(ver, nullptr, nullptr, nullptr);
    if (!mPng)
      return;

    mPngInfo = png_create_info_struct(mPng);
    if (!mPngInfo)
      return;

    if (setjmp(png_jmpbuf(mPng)))
      return;

    png_init_io(mPng, mFile);

    png_set_compression_level(mPng, Z_BEST_COMPRESSION);

    auto bits = (kind == PngKind::Height) ? 16 : 8;

    auto colorType =
      (kind == PngKind::Height) ? PNG_COLOR_TYPE_GRAY : PNG_COLOR_TYPE_RGB;

    png_set_IHDR(mPng,
                 mPngInfo,
                 w,
                 h,
                 bits,
                 colorType,
                 PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_DEFAULT,
                 PNG_FILTER_TYPE_DEFAULT);

    png_write_info(mPng, mPngInfo);
  }

  ~PngRowStream()
  {
    if (mFile && mPng && mPngInfo) {
      EndWrite();
    }

    if (mPng)
      png_destroy_write_struct(&mPng, &mPngInfo);

    if (mFile)
      fclose(mFile);
  }

  void WriteRow(const unsigned char* data)
  {
    if (setjmp(png_jmpbuf(mPng)))
      return;

    png_write_row(mPng, data);
  }

private:
  void EndWrite()
  {
    if (setjmp(png_jmpbuf(mPng)))
      return;

    png_write_end(mPng, mPngInfo);
  }

private:
  FILE* mFile = nullptr;

  png_structp mPng = nullptr;

  png_infop mPngInfo = nullptr;
};

class PngWriterImpl final : public PngWriter
{
public:
  PngWriterImpl(size_t w,
                size_t h,
                const char* heightPath,
                const char* colorPath)
    : mWidth(w)
    , mHeight(h)
    , mHeightStream(heightPath, w, h, PngKind::Height)
    , mColorStream(colorPath, w, h, PngKind::Color)
    , mHeightBuffer16Bit(w * 2)
    , mColorBuffer(w * 3)
  {}

  void Observe(const float* heightAndRgbData) override
  {
    auto heightRange = mHeightMax - mHeightMin;

    for (size_t i = 0; i < mWidth; i++) {

      auto height = (heightAndRgbData[i * 4] - mHeightMin) / heightRange;

      auto height16 = uint16_t(height * std::numeric_limits<uint16_t>::max());

      mHeightBuffer16Bit[(i * 2) + 0] = height16 >> 8;
      mHeightBuffer16Bit[(i * 2) + 1] = height16 >> 0;

      mColorBuffer[(i * 3) + 0] = heightAndRgbData[(i * 4) + 1];
      mColorBuffer[(i * 3) + 1] = heightAndRgbData[(i * 4) + 2];
      mColorBuffer[(i * 3) + 2] = heightAndRgbData[(i * 4) + 3];
    }

    mHeightStream.WriteRow(mHeightBuffer16Bit.data());

    mColorStream.WriteRow(mColorBuffer.data());
  }

  void SetHeightRange(float min, float max)
  {
    mHeightMin = min;
    mHeightMax = max;
  }

private:
  static float Clamp(float x, float min, float max) noexcept
  {
    return std::min(std::max(x, min), max);
  }

private:
  size_t mWidth;

  size_t mHeight;

  PngRowStream mHeightStream;

  PngRowStream mColorStream;

  float mHeightMin = 0;

  float mHeightMax = 1;

  std::vector<uint8_t> mHeightBuffer16Bit;

  std::vector<uint8_t> mColorBuffer;
};

} // namespace

auto
PngWriter::Make(size_t w,
                size_t h,
                const char* heightPath,
                const char* colorPath) -> std::unique_ptr<PngWriter>
{
  using Ret = std::unique_ptr<PngWriter>;

  return Ret(new PngWriterImpl(w, h, heightPath, colorPath));
}

} // namespace terra
