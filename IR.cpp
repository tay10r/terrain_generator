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

} // namespace ir
