#include "core/CpuBackend.h"

#include "core/HeightMapObserver.h"
#include "core/IR.h"

#include <fstream>
#include <iostream>
#include <vector>

#include <math.h>
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

class BinaryFloatExpr : public FloatExpr
{
public:
  BinaryFloatExpr(std::unique_ptr<FloatExpr> l, std::unique_ptr<FloatExpr> r)
    : mLeft(std::move(l))
    , mRight(std::move(r))
  {}

  virtual ~BinaryFloatExpr() = default;

protected:
  float EvalLeft(const BuiltinVars& builtins) const noexcept
  {
    return mLeft->Eval(builtins);
  }

  float EvalRight(const BuiltinVars& builtins) const noexcept
  {
    return mRight->Eval(builtins);
  }

private:
  std::unique_ptr<FloatExpr> mLeft;

  std::unique_ptr<FloatExpr> mRight;
};

class AddFloatExpr final : public BinaryFloatExpr
{
public:
  using BinaryFloatExpr::BinaryFloatExpr;

  float Eval(const BuiltinVars& builtins) const noexcept override
  {
    return EvalLeft(builtins) + EvalRight(builtins);
  }
};

class SubFloatExpr final : public BinaryFloatExpr
{
public:
  using BinaryFloatExpr::BinaryFloatExpr;

  float Eval(const BuiltinVars& builtins) const noexcept override
  {
    return EvalLeft(builtins) - EvalRight(builtins);
  }
};

class MulFloatExpr final : public BinaryFloatExpr
{
public:
  using BinaryFloatExpr::BinaryFloatExpr;

  float Eval(const BuiltinVars& builtins) const noexcept override
  {
    return EvalLeft(builtins) * EvalRight(builtins);
  }
};

class DivFloatExpr final : public BinaryFloatExpr
{
public:
  using BinaryFloatExpr::BinaryFloatExpr;

  float Eval(const BuiltinVars& builtins) const noexcept override
  {
    return EvalLeft(builtins) / EvalRight(builtins);
  }
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

class UnaryFloatExpr : public FloatExpr
{
public:
  UnaryFloatExpr(std::unique_ptr<FloatExpr> innerExpr)
    : mInnerExpr(std::move(innerExpr))
  {}

protected:
  float InnerEval(const BuiltinVars& builtins) const noexcept
  {
    return mInnerExpr->Eval(builtins);
  }

private:
  std::unique_ptr<FloatExpr> mInnerExpr;
};

class SineExpr final : public UnaryFloatExpr
{
public:
  using UnaryFloatExpr::UnaryFloatExpr;

  float Eval(const BuiltinVars& builtins) const noexcept override
  {
    return sin(InnerEval(builtins));
  }
};

class CosineExpr final : public UnaryFloatExpr
{
public:
  using UnaryFloatExpr::UnaryFloatExpr;

  float Eval(const BuiltinVars& builtins) const noexcept override
  {
    return cos(InnerEval(builtins));
  }
};

class TangentExpr final : public UnaryFloatExpr
{
public:
  using UnaryFloatExpr::UnaryFloatExpr;

  float Eval(const BuiltinVars& builtins) const noexcept override
  {
    return tan(InnerEval(builtins));
  }
};

class ArcsineExpr final : public UnaryFloatExpr
{
public:
  using UnaryFloatExpr::UnaryFloatExpr;

  float Eval(const BuiltinVars& builtins) const noexcept override
  {
    return asin(InnerEval(builtins));
  }
};

class ArccosineExpr final : public UnaryFloatExpr
{
public:
  using UnaryFloatExpr::UnaryFloatExpr;

  float Eval(const BuiltinVars& builtins) const noexcept override
  {
    return acos(InnerEval(builtins));
  }
};

class ArctangentExpr final : public UnaryFloatExpr
{
public:
  using UnaryFloatExpr::UnaryFloatExpr;

  float Eval(const BuiltinVars& builtins) const noexcept override
  {
    return atan(InnerEval(builtins));
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

  void Visit(const ir::UnaryTrigExpr&) override {}

  void Visit(const ir::BinaryExpr& binaryExpr) override
  {
    // TODO
    (void)binaryExpr;
  }

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

  void Visit(const ir::UnaryTrigExpr& trigExpr) override
  {
    FloatExprBuilder subBuilder;

    trigExpr.GetInputExpr().Accept(subBuilder);

    auto operand = subBuilder.TakeResult();

    if (!operand)
      return;

    switch (trigExpr.GetID()) {
      case ir::UnaryTrigExpr::ID::Sine:
        mFloatExpr.reset(new SineExpr(std::move(operand)));
        break;
      case ir::UnaryTrigExpr::ID::Cosine:
        mFloatExpr.reset(new CosineExpr(std::move(operand)));
        break;
      case ir::UnaryTrigExpr::ID::Tangent:
        mFloatExpr.reset(new TangentExpr(std::move(operand)));
        break;
      case ir::UnaryTrigExpr::ID::Arcsine:
        mFloatExpr.reset(new ArcsineExpr(std::move(operand)));
        break;
      case ir::UnaryTrigExpr::ID::Arccosine:
        mFloatExpr.reset(new ArccosineExpr(std::move(operand)));
        break;
      case ir::UnaryTrigExpr::ID::Arctangent:
        mFloatExpr.reset(new ArctangentExpr(std::move(operand)));
        break;
    }
  }

  void Visit(const ir::IntLiteralExpr&) override {}

  void Visit(const ir::FloatToIntExpr&) override {}

  void Visit(const ir::BinaryExpr& binaryExpr) override
  {
    auto lExpr = BuildSubExpr(binaryExpr.GetLeftExpr());
    auto rExpr = BuildSubExpr(binaryExpr.GetRightExpr());

    if (!lExpr || !rExpr)
      return;

    switch (binaryExpr.GetID()) {
      case ir::BinaryExpr::ID::Add:
        mFloatExpr.reset(new AddFloatExpr(std::move(lExpr), std::move(rExpr)));
        break;
      case ir::BinaryExpr::ID::Sub:
        mFloatExpr.reset(new SubFloatExpr(std::move(lExpr), std::move(rExpr)));
        break;
      case ir::BinaryExpr::ID::Mul:
        mFloatExpr.reset(new MulFloatExpr(std::move(lExpr), std::move(rExpr)));
        break;
      case ir::BinaryExpr::ID::Div:
        mFloatExpr.reset(new DivFloatExpr(std::move(lExpr), std::move(rExpr)));
        break;
    }
  }

private:
  static auto BuildSubExpr(const ir::Expr& expr) -> std::unique_ptr<FloatExpr>
  {
    FloatExprBuilder builder;

    expr.Accept(builder);

    return builder.TakeResult();
  }

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
