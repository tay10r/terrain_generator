#include "IR.h"

namespace ir {

VarRefExpr::VarRefExpr(ID id) noexcept
  : mID(id)
{}

void
VarRefExpr::Accept(ExprVisitor& visitor) const
{
  visitor.Visit(*this);
}

auto
VarRefExpr::GetID() const noexcept -> ID
{
  return mID;
}

auto
VarRefExpr::GetType() const noexcept -> std::optional<Type>
{
  switch (mID) {
    case ID::CenterUCoord:
    case ID::CenterVCoord:
      return Type::Float;
  }

  return {};
}

CastExpr::CastExpr(std::unique_ptr<Expr>&& sourceExpr)
  : mSourceExpr(std::move(sourceExpr))
{}

auto
CastExpr::GetSourceExpr() const noexcept -> const Expr&
{
  return *mSourceExpr;
}

auto
IntToFloatExpr::GetType() const noexcept -> std::optional<Type>
{
  return Type::Float;
}

void
IntToFloatExpr::Accept(ExprVisitor& visitor) const
{
  visitor.Visit(*this);
}

auto
FloatToIntExpr::GetType() const noexcept -> std::optional<Type>
{
  return Type::Int;
}

void
FloatToIntExpr::Accept(ExprVisitor& visitor) const
{
  visitor.Visit(*this);
}

} // namespace ir
