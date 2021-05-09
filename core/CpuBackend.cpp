#include "core/CpuBackend.h"

#include "core/HeightMapObserver.h"
#include "core/IR.h"

#include <fstream>
#include <iostream>
#include <vector>

#include <stdint.h>

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

class FloatLiteral final : public FloatExpr
{
public:
  FloatLiteral(float value)
    : mValue(value)
  {}

  float Eval(const BuiltinVars&) const noexcept override { return mValue; }

private:
  float mValue;
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

class IntExpr
{
public:
  virtual ~IntExpr() = default;

  virtual int Eval(const BuiltinVars& builtinVars) const noexcept = 0;
};

class IntToFloatExpr final : public FloatExpr
{
public:
  IntToFloatExpr(std::unique_ptr<IntExpr>&& intExpr)
    : mIntExpr(std::move(intExpr))
  {}

  float Eval(const BuiltinVars& builtinVars) const noexcept override
  {
    return mIntExpr->Eval(builtinVars);
  }

private:
  std::unique_ptr<IntExpr> mIntExpr;
};

class IntLiteral final : public IntExpr
{
public:
  IntLiteral(int value)
    : mValue(value)
  {}

  int Eval(const BuiltinVars&) const noexcept override { return mValue; }

private:
  int mValue;
};

class FloatToInt final : public IntExpr
{
public:
  FloatToInt(std::unique_ptr<FloatExpr>&& floatExpr)
    : mFloatExpr(std::move(floatExpr))
  {}

  int Eval(const BuiltinVars& builtinVars) const noexcept override
  {
    return mFloatExpr->Eval(builtinVars);
  }

private:
  std::unique_ptr<FloatExpr> mFloatExpr;
};

class IntExprBuilder final : public ir::ExprVisitor
{
public:
  auto TakeResult() -> std::unique_ptr<IntExpr> { return std::move(mExpr); }

  void Visit(const ir::FloatLiteralExpr&) override {}

  void Visit(const ir::IntLiteralExpr& literalExpr) override
  {
    mExpr.reset(new IntLiteral(literalExpr.GetValue()));
  }

  void Visit(const ir::VarRefExpr&) override {}

  void Visit(const ir::IntToFloatExpr&) override {}

  void Visit(const ir::FloatToIntExpr& floatToInt) override;

private:
  std::unique_ptr<IntExpr> mExpr;
};

class FloatExprBuilder final : public ir::ExprVisitor
{
public:
  auto TakeResult() -> std::unique_ptr<FloatExpr>
  {
    return std::move(mFloatExpr);
  }

  void Visit(const ir::VarRefExpr& varRefExpr) override
  {
    switch (varRefExpr.GetID()) {
      case ir::VarRefExpr::ID::CenterUCoord:
        mFloatExpr.reset(new UCoordExpr());
        break;
      case ir::VarRefExpr::ID::CenterVCoord:
        mFloatExpr.reset(new VCoordExpr());
        break;
    }
  }

  void Visit(const ir::FloatLiteralExpr& floatLiteralExpr) override
  {
    mFloatExpr.reset(new FloatLiteral(floatLiteralExpr.GetValue()));
  }

  void Visit(const ir::IntToFloatExpr& expr) override
  {
    IntExprBuilder intExprBuilder;

    expr.GetSourceExpr().Accept(intExprBuilder);

    auto intExpr = intExprBuilder.TakeResult();

    if (!intExpr)
      return;

    mFloatExpr.reset(new IntToFloatExpr(std::move(intExpr)));
  }

  void Visit(const ir::IntLiteralExpr&) override {}

  void Visit(const ir::FloatToIntExpr&) override {}

private:
  std::unique_ptr<FloatExpr> mFloatExpr;
};

void
IntExprBuilder::Visit(const ir::FloatToIntExpr& floatToInt)
{
  FloatExprBuilder floatExprBuilder;

  floatToInt.GetSourceExpr().Accept(floatExprBuilder);

  auto floatExpr = floatExprBuilder.TakeResult();

  if (!floatExpr)
    return;

  mExpr.reset(new FloatToInt(std::move(floatExpr)));
}

class CpuBackendImpl final : public CpuBackend
{
public:
  CpuBackendImpl() {}

  void ComputeHeightMap() override
  {
    if (!mHeightMapExpr)
      mHeightMapExpr.reset(new FloatLiteral(0.0));

    BuiltinVars builtinVars;

    for (size_t i = 0; i < (mWidth * mHeight); i++) {

      size_t x = i % mWidth;
      size_t y = i / mWidth;

      builtinVars.u = (x + 0.5f) / mWidth;
      builtinVars.v = (y + 0.5f) / mHeight;

      mHeightMap[i] = mHeightMapExpr->Eval(builtinVars);
    }

    for (auto& observer : mHeightMapObservers)
      observer->Observe(mHeightMap.data(), mWidth, mHeight);
  }

  void ComputeSurface() override
  {
    // TODO
  }

  void Resize(size_t w, size_t h) override
  {
    mHeightMap.resize(w * h);

    mWidth = w;

    mHeight = h;
  }

  bool UpdateHeightExpr(const ir::Expr* expr) override
  {
    if (!expr) {
      mHeightMapExpr = std::unique_ptr<FloatExpr>(new FloatLiteral(0.0f));
      return false;
    }

    FloatExprBuilder floatExprBuilder;

    expr->Accept(floatExprBuilder);

    auto result = floatExprBuilder.TakeResult();

    if (!result) {
      mHeightMapExpr = std::unique_ptr<FloatExpr>(new FloatLiteral(0.0f));
      return false;
    }

    mHeightMapExpr = std::move(result);

    return true;
  }

  bool UpdateColorExpr(const ir::Expr* expr) override
  {
    (void)expr;
    // TODO
    return false;
  }

  void ReadHeightMap(float* buf) const override
  {
    for (size_t i = 0; i < mHeightMap.size(); i++)
      buf[i] = mHeightMap[i];
  }

  void AddHeightMapObserver(
    std::unique_ptr<HeightMapObserver> observer) override
  {
    mHeightMapObservers.emplace_back(std::move(observer));
  }

private:
  std::vector<std::unique_ptr<HeightMapObserver>> mHeightMapObservers;

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
