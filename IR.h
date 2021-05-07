#pragma once

#include <optional>

namespace ir {

enum class Type
{
  Float
};

class VarRefExpr;

class ExprVisitor
{
public:
  virtual ~ExprVisitor() = default;

  virtual void Visit(const VarRefExpr&) = 0;
};

class Expr
{
public:
  virtual ~Expr() = default;

  virtual void Accept(ExprVisitor&) const = 0;

  virtual auto GetType() const noexcept -> std::optional<Type> = 0;
};

class VarRefExpr final : public Expr
{
public:
  enum class ID
  {
    CenterUCoord,
    CenterVCoord
  };

  VarRefExpr(ID id) noexcept;

  void Accept(ExprVisitor& visitor) const override;

  auto GetID() const noexcept -> ID;

  auto GetType() const noexcept -> std::optional<Type>;

private:
  ID mID;
};

} // namespace ir
