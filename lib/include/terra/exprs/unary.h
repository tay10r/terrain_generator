#pragma once

#include <terra/expr.h>

#include <memory>

namespace terra {

class UnaryExpr final : public Expr
{
public:
  enum class ID
  {
    Sine,
    Cosine,
    Tangent,
    Arcsine,
    Arccosine,
    Arctangent
  };

  UnaryExpr(ID id, std::shared_ptr<Expr> inputExpr);

  void Accept(ExprVisitor& visitor) const override;

  auto GetType() const noexcept -> std::optional<Type> override;

  auto GetID() const noexcept -> ID;

  auto GetInputExpr() const noexcept -> const Expr&;

private:
  ID mID;

  std::shared_ptr<Expr> mInputExpr;
};

} // namespace terra
