#include <terra/exprs/unary.h>

#include <terra/expr_visitor.h>

namespace terra {

UnaryExpr::UnaryExpr(ID id, std::shared_ptr<Expr> inputExpr)
  : mID(id)
  , mInputExpr(inputExpr)
{}

void
UnaryExpr::Accept(ExprVisitor& v) const
{
  v.Visit(*this);
}

auto
UnaryExpr::GetType() const noexcept -> std::optional<Type>
{
  switch (mID) {
    case ID::Sine:
    case ID::Cosine:
    case ID::Tangent:
    case ID::Arcsine:
    case ID::Arccosine:
    case ID::Arctangent:
      return Type::Float;
  }

  return {};
}

auto
UnaryExpr::GetID() const noexcept -> ID
{
  return mID;
}

auto
UnaryExpr::GetInputExpr() const noexcept -> const Expr&
{
  return *mInputExpr;
}

} // namespace terra
