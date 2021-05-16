#pragma once

#include <terra/expr.h>

#include <array>
#include <memory>

namespace terra {

template<size_t Size>
class VectorCombiner final : public Expr
{
public:
  VectorCombiner(std::array<std::shared_ptr<Expr>, Size> exprs)
    : mExprs(exprs)
  {}

  void Accept(ExprVisitor& visitor) const { visitor.Visit(*this); }

  auto GetElement(size_t index) const noexcept -> const Expr&
  {
    return *mExprs[index];
  }

  auto GetType() const noexcept -> std::optional<Type>
  {
    if (!AllElementsAreFloat())
      return {};

    switch (Size) {
      case 2:
        return Type::Vec2;
      case 3:
        return Type::Vec3;
      case 4:
        return Type::Vec4;
    }

    return {};
  }

private:
  bool AllElementsAreFloat() const noexcept
  {
    for (const auto& expr : mExprs) {
      if (expr->GetType() != Type::Float)
        return false;
    }

    return true;
  }

  std::array<std::shared_ptr<Expr>, Size> mExprs;
};

} // namespace terra
