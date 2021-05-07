#pragma once

#include <optional>
#include <vector>

namespace ir {

enum class Type
{
  Float,
  Int
};

class VarRefExpr;

template<typename ValueType>
class LiteralExpr;

using IntLiteralExpr = LiteralExpr<int>;

using FloatLiteralExpr = LiteralExpr<float>;

class ExprVisitor
{
public:
  virtual ~ExprVisitor() = default;

  virtual void Visit(const VarRefExpr&) = 0;

  virtual void Visit(const IntLiteralExpr&) = 0;

  virtual void Visit(const FloatLiteralExpr&) = 0;
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

template<typename ValueType>
struct TypeMap final
{};

template<>
struct TypeMap<int> final
{
  static constexpr Type GetType() noexcept { return Type::Int; }
};

template<>
struct TypeMap<float> final
{
  static constexpr Type GetType() noexcept { return Type::Float; }
};

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

} // namespace ir
