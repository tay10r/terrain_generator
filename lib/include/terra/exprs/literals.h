#pragma once

#include <terra/expr.h>

#include <terra/expr_visitor.h>

namespace terra {

template<typename ValueType>
class LiteralExpr final : public Expr
{
public:
  LiteralExpr(ValueType value)
    : mValue(value)
  {}

  void Accept(ExprVisitor& visitor) const override { visitor.Visit(*this); }

  auto GetValue() const noexcept { return mValue; }

  auto GetType() const noexcept -> std::optional<Type> override
  {
    return TypeMap<ValueType>::GetType();
  }

private:
  ValueType mValue;
};

} // namespace terra
