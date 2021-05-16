#pragma once

#include <terra/type.h>

#include <optional>

namespace terra {

class ExprVisitor;

class Expr
{
public:
  virtual ~Expr() = default;

  virtual void Accept(ExprVisitor&) const = 0;

  virtual auto GetType() const noexcept -> std::optional<Type> = 0;
};

} // namespace terra
