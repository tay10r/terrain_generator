#include "ExprNodeData.h"

#include "core/IR.h"

#include <nodes/NodeDataModel>

namespace {

class ExprNodeData final : public QtNodes::NodeData
{
public:
  ExprNodeData(ir::Expr* expr)
    : mExpr(expr)
  {}

  QtNodes::NodeDataType type() const override
  {
    auto exprType = mExpr->GetType();

    if (!exprType)
      return QtNodes::NodeDataType{ "", "" };

    switch (*exprType) {
      case ir::Type::Float:
        return QtNodes::NodeDataType{ "float", "" };
      case ir::Type::Int:
        return QtNodes::NodeDataType{ "int", "" };
      case ir::Type::Vec2:
        return QtNodes::NodeDataType{ "vec2", "" };
      case ir::Type::Vec3:
        return QtNodes::NodeDataType{ "vec3", "" };
      case ir::Type::Vec4:
        return QtNodes::NodeDataType{ "vec4", "" };
      case ir::Type::Mat2:
        return QtNodes::NodeDataType{ "mat2", "" };
      case ir::Type::Mat3:
        return QtNodes::NodeDataType{ "mat3", "" };
      case ir::Type::Mat4:
        return QtNodes::NodeDataType{ "mat4", "" };
    }

    return QtNodes::NodeDataType{ "", "" };
  }

  const ir::Expr* GetExpr() const noexcept { return mExpr.get(); }

private:
  std::unique_ptr<ir::Expr> mExpr;
};

} // namespace

auto
ExprToNodeData(ir::Expr* expr) -> std::shared_ptr<QtNodes::NodeData>
{
  return std::make_shared<ExprNodeData>(expr);
}

auto
NodeDataToExpr(const QtNodes::NodeData* nodeData) -> const ir::Expr*
{
  const auto* asExprNode = dynamic_cast<const ExprNodeData*>(nodeData);
  if (!asExprNode)
    return nullptr;
  else
    return asExprNode->GetExpr();
}
