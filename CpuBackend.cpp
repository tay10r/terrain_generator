#include "CpuBackend.h"

#include "Expr.h"

#include <vector>

#include <fstream>

namespace {

struct BuiltinVars final
{
  float u;
  float v;
};

class FloatExpr
{
public:
  virtual ~FloatExpr() = default;

  virtual float Eval(const BuiltinVars&) const noexcept = 0;
};

class UCoordExpr final : public FloatExpr
{
public:
  float Eval(const BuiltinVars& builtinVars) const noexcept override
  {
    return builtinVars.u;
  }
};

class VCoordExpr final : public FloatExpr
{
public:
  float Eval(const BuiltinVars& builtinVars) const noexcept override
  {
    return builtinVars.v;
  }
};

class FloatExprBuilder : public ir::ExprVisitor
{
public:
  auto TakeResult() -> std::unique_ptr<FloatExpr>
  {
    return std::move(mFloatExpr);
  }

  void Visit(const ir::VarRefExpr& varRefExpr) override
  {
    switch (varRefExpr.GetID()) {
      case ir::VarRefExpr::ID::XCoord:
        mFloatExpr.reset(new UCoordExpr());
        break;
      case ir::VarRefExpr::ID::YCoord:
        mFloatExpr.reset(new VCoordExpr());
        break;
    }
  }

private:
  std::unique_ptr<FloatExpr> mFloatExpr;
};

class CpuBackendImpl final : public CpuBackend
{
public:
  void ComputeHeightMap() override
  {
    BuiltinVars builtinVars;

    for (size_t i = 0; i < (mWidth * mHeight); i++) {

      size_t x = i % mWidth;
      size_t y = i / mWidth;

      builtinVars.u = (x + 0.5f) / mWidth;
      builtinVars.v = (y + 0.5f) / mHeight;

      mHeightMap[i] = mHeightMapExpr->Eval(builtinVars);
    }
  }

  void Display() override
  {
    std::ofstream file("image.ppm");
    file << "P6" << std::endl;
    file << mWidth << ' ' << mHeight << std::endl;
    file << "255" << std::endl;

    using uchar = unsigned char;

    std::vector<uchar> c(mWidth * mHeight * 3);

    for (size_t i = 0; i < (mWidth * mHeight); i++) {

      auto v = uchar(255 * mHeightMap[i]);

      c[(i * 3) + 0] = v;
      c[(i * 3) + 1] = v;
      c[(i * 3) + 2] = v;
    }

    file.write((const char*)c.data(), c.size());
  }

  void Resize(size_t w, size_t h) override
  {
    mHeightMap.resize(w * h);

    mWidth = w;

    mHeight = h;
  }

  bool UpdateHeightMapExpr(const ir::Expr& expr) override
  {
    FloatExprBuilder floatExprBuilder;

    expr.Accept(floatExprBuilder);

    auto result = floatExprBuilder.TakeResult();

    if (!result)
      return false;

    mHeightMapExpr = std::move(result);

    return true;
  }

private:
  std::unique_ptr<FloatExpr> mHeightMapExpr;

  std::vector<float> mHeightMap;

  size_t mWidth = 0;

  size_t mHeight = 0;
};

} // namespace

auto
CpuBackend::Make() -> CpuBackend*
{
  return new CpuBackendImpl();
}
