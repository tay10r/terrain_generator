#pragma once

#include <memory>
#include <optional>
#include <vector>

namespace ir {

enum class Type
{
  Float,
  Int,
  Vec2,
  Vec3,
  Vec4,
  Mat2,
  Mat3,
  Mat4
};

class VarRefExpr;
class IntToFloatExpr;
class FloatToIntExpr;
class UnaryTrigExpr;

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

  virtual void Visit(const FloatToIntExpr&) = 0;

  virtual void Visit(const IntToFloatExpr&) = 0;

  virtual void Visit(const UnaryTrigExpr&) = 0;
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

  auto GetType() const noexcept -> std::optional<Type> override;

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

class UnaryTrigExpr final : public Expr
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

  UnaryTrigExpr(ID id, const Expr& inputExpr)
    : mID(id)
    , mInputExpr(inputExpr)
  {}

  void Accept(ExprVisitor& visitor) const override { visitor.Visit(*this); }

  auto GetType() const noexcept -> std::optional<Type> { return Type::Float; }

  auto GetID() const noexcept { return mID; }

  auto GetInputExpr() const noexcept -> const Expr& { return mInputExpr; }

private:
  ID mID;

  const Expr& mInputExpr;
};

} // namespace ir
