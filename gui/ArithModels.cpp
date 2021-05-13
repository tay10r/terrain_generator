#include "ArithModels.h"

#include "core/IR.h"

#include "gui/ExprNodeData.h"

#include <nodes/DataModelRegistry>
#include <nodes/NodeDataModel>

namespace {

class BinaryArithModel : public QtNodes::NodeDataModel
{
public:
  virtual ~BinaryArithModel() = default;

  virtual ir::BinaryExpr::ID GetID() const = 0;

  ir::Expr* MakeExpr(const ir::Expr& l, const ir::Expr& r) const
  {
    return new ir::BinaryExpr(GetID(), l, r);
  }

  unsigned int nPorts(QtNodes::PortType portType) const override
  {
    switch (portType) {
      case QtNodes::PortType::None:
        break;
      case QtNodes::PortType::In:
        return 2;
      case QtNodes::PortType::Out:
        return 1;
    }

    return 0;
  }

  auto outData(QtNodes::PortIndex)
    -> std::shared_ptr<QtNodes::NodeData> override
  {
    if (!mLeftNodeData || !mRightNodeData)
      return nullptr;

    const auto* lExpr = NodeDataToExpr(mLeftNodeData.get());

    const auto* rExpr = NodeDataToExpr(mRightNodeData.get());

    return ExprToNodeData(MakeExpr(*lExpr, *rExpr));
  }

  auto dataType(QtNodes::PortType portType, QtNodes::PortIndex) const
    -> QtNodes::NodeDataType override
  {
    switch (portType) {
      case QtNodes::PortType::None:
        break;
      case QtNodes::PortType::In:
        return QtNodes::NodeDataType{ "float", "Input" };
      case QtNodes::PortType::Out:
        return QtNodes::NodeDataType{ "float", "Output" };
    }

    return QtNodes::NodeDataType{ "", "" };
  }

  void setInData(std::shared_ptr<QtNodes::NodeData> nodeData,
                 QtNodes::PortIndex portIndex) override
  {
    switch (portIndex) {
      case 0:
        mLeftNodeData = nodeData;
        break;
      case 1:
        mRightNodeData = nodeData;
        break;
    }

    emit dataUpdated(0);
  }

  auto embeddedWidget() -> QWidget* override { return nullptr; }

private:
  std::shared_ptr<QtNodes::NodeData> mLeftNodeData;

  std::shared_ptr<QtNodes::NodeData> mRightNodeData;
};

class AddModel final : public BinaryArithModel
{
public:
  QString caption() const override { return QStringLiteral("Add"); }

  QString name() const override { return QStringLiteral("Add"); }

  ir::BinaryExpr::ID GetID() const override { return ir::BinaryExpr::ID::Add; }
};

class SubModel final : public BinaryArithModel
{
public:
  QString caption() const override { return QStringLiteral("Subtract"); }

  QString name() const override { return QStringLiteral("Subtract"); }

  ir::BinaryExpr::ID GetID() const override { return ir::BinaryExpr::ID::Sub; }
};

class MulModel final : public BinaryArithModel
{
public:
  QString caption() const override { return QStringLiteral("Multiply"); }

  QString name() const override { return QStringLiteral("Multiply"); }

  ir::BinaryExpr::ID GetID() const override { return ir::BinaryExpr::ID::Mul; }
};

class DivModel final : public BinaryArithModel
{
public:
  QString caption() const override { return QStringLiteral("Divide"); }

  QString name() const override { return QStringLiteral("Divide"); }

  ir::BinaryExpr::ID GetID() const override { return ir::BinaryExpr::ID::Div; }
};

} // namespace

void
DefineArithModels(QtNodes::DataModelRegistry& registry)
{
  registry.registerModel<AddModel>("Arithmetic");
  registry.registerModel<SubModel>("Arithmetic");
  registry.registerModel<MulModel>("Arithmetic");
  registry.registerModel<DivModel>("Arithmetic");
}
