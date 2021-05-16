#pragma once

#include <terra/expr.h>

namespace terra {

class CastExpr : public Expr
{
public:
  CastExpr(std::unique_ptr<Expr>&& sourceExpr);

  const Expr& GetSourceExpr() const noexcept;

private:
  std::unique_ptr<Expr> mSourceExpr;
};

class IntToFloatExpr final : public CastExpr
{
public:
  using CastExpr::CastExpr;

  void Accept(ExprVisitor& visitor) const override;

  auto GetType() const noexcept -> std::optional<Type> override;
};

class FloatToIntExpr final : public CastExpr
{
public:
  using CastExpr::CastExpr;

  void Accept(ExprVisitor& visitor) const override;

  auto GetType() const noexcept -> std::optional<Type> override;
};

} // namespace terra
