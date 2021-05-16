#pragma once

#include <terra/expr.h>

namespace terra {

class VarRefExpr final : public Expr
{
public:
  enum class ID
  {
    CenterU,
    CenterV
  };

  VarRefExpr(ID id) noexcept;

  void Accept(ExprVisitor& visitor) const override;

  auto GetID() const noexcept -> ID;

  auto GetType() const noexcept -> std::optional<Type> override;

private:
  ID mID;
};

} // namespace terra
