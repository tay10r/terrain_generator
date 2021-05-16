#include <terra/exprs/var_ref.h>

#include <terra/expr_visitor.h>

namespace terra {

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
    case ID::CenterU:
    case ID::CenterV:
      return Type::Float;
  }

  return {};
}

} // namespace terra
