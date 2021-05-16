#pragma once

namespace terra {

class VarRefExpr;
class IntToFloatExpr;
class FloatToIntExpr;
class UnaryExpr;
class BinaryExpr;

template<typename ValueType>
class LiteralExpr;

using IntLiteralExpr = LiteralExpr<int>;

using FloatLiteralExpr = LiteralExpr<float>;

template<size_t>
class VectorCombiner;

class ExprVisitor
{
public:
  virtual ~ExprVisitor() = default;

  virtual void Visit(const VarRefExpr&) = 0;

  virtual void Visit(const IntLiteralExpr&) = 0;

  virtual void Visit(const FloatLiteralExpr&) = 0;

  virtual void Visit(const FloatToIntExpr&) = 0;

  virtual void Visit(const IntToFloatExpr&) = 0;

  virtual void Visit(const UnaryExpr&) = 0;

  virtual void Visit(const BinaryExpr&) = 0;

  virtual void Visit(const VectorCombiner<2>&) = 0;

  virtual void Visit(const VectorCombiner<3>&) = 0;

  virtual void Visit(const VectorCombiner<4>&) = 0;
};

} // namespace terra
