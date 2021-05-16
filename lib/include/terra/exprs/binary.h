#pragma once

#include <terra/expr.h>

#include <memory>

namespace terra {

class BinaryExpr final : public Expr
{
public:
  enum class ID
  {
    Add,
    Sub,
    Mul,
    Div
  };

  BinaryExpr(ID id, std::shared_ptr<Expr> left, std::shared_ptr<Expr> right)
    : mID(id)
    , mLeft(left)
    , mRight(right)
  {}

  void Accept(ExprVisitor& visitor) const override { visitor.Visit(*this); }

  auto GetID() const noexcept -> ID { return mID; }

  auto GetLeftExpr() const noexcept -> const Expr& { return *mLeft; }

  auto GetRightExpr() const noexcept -> const Expr& { return *mRight; }

  auto GetType() const noexcept -> std::optional<Type> override;

private:
  ID mID;

  std::shared_ptr<Expr> mLeft;

  std::shared_ptr<Expr> mRight;
};

} // namespace terra
