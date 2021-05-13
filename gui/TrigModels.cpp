#include "TrigModels.h"

#include "core/IR.h"

#include "gui/ExprNodeData.h"

#include <nodes/DataModelRegistry>
#include <nodes/NodeDataModel>

namespace {

class UnaryTrigModel : public QtNodes::NodeDataModel
{
public:
  virtual ~UnaryTrigModel() = default;

  virtual ir::UnaryTrigExpr::ID GetID() const = 0;

  ir::Expr* MakeExpr(const ir::Expr& input) const
  {
    return new ir::UnaryTrigExpr(GetID(), input);
  }

  unsigned int nPorts(QtNodes::PortType portType) const override
  {
    switch (portType) {
      case QtNodes::PortType::None:
        break;
      case QtNodes::PortType::In:
        return 1;
      case QtNodes::PortType::Out:
        return 1;
    }

    return 0;
  }

  auto outData(QtNodes::PortIndex)
    -> std::shared_ptr<QtNodes::NodeData> override
  {
    if (!mInputNodeData)
      return nullptr;

    const auto* expr = NodeDataToExpr(mInputNodeData.get());

    return ExprToNodeData(MakeExpr(*expr));
  }

  auto dataType(QtNodes::PortType portType, QtNodes::PortIndex) const
    -> QtNodes::NodeDataType override
  {
    const char* inName = "Angle";

    const char* outName = "Output";

    switch (GetID()) {
      case ir::UnaryTrigExpr::ID::Sine:
      case ir::UnaryTrigExpr::ID::Cosine:
      case ir::UnaryTrigExpr::ID::Tangent:
        break;
      case ir::UnaryTrigExpr::ID::Arcsine:
      case ir::UnaryTrigExpr::ID::Arccosine:
      case ir::UnaryTrigExpr::ID::Arctangent:
        inName = "Input";
        outName = "Angle";
        break;
    }

    switch (portType) {
      case QtNodes::PortType::None:
        break;
      case QtNodes::PortType::In:
        return QtNodes::NodeDataType{ "float", inName };
      case QtNodes::PortType::Out:
        return QtNodes::NodeDataType{ "float", outName };
    }

    return QtNodes::NodeDataType{ "", "" };
  }

  void setInData(std::shared_ptr<QtNodes::NodeData> nodeData,
                 QtNodes::PortIndex) override
  {
    mInputNodeData = nodeData;

    emit dataUpdated(0);
  }

  auto embeddedWidget() -> QWidget* override { return nullptr; }

private:
  std::shared_ptr<QtNodes::NodeData> mInputNodeData;
};

class SineModel final : public UnaryTrigModel
{
public:
  QString caption() const override { return QStringLiteral("sin"); }

  QString name() const override { return QStringLiteral("sin"); }

  ir::UnaryTrigExpr::ID GetID() const override
  {
    return ir::UnaryTrigExpr::ID::Sine;
  }
};

class CosineModel final : public UnaryTrigModel
{
public:
  QString caption() const override { return QStringLiteral("cos"); }

  QString name() const override { return QStringLiteral("cos"); }

  ir::UnaryTrigExpr::ID GetID() const override
  {
    return ir::UnaryTrigExpr::ID::Cosine;
  }
};

class TangentModel final : public UnaryTrigModel
{
public:
  QString caption() const override { return QStringLiteral("tan"); }

  QString name() const override { return QStringLiteral("tan"); }

  ir::UnaryTrigExpr::ID GetID() const override
  {
    return ir::UnaryTrigExpr::ID::Tangent;
  }
};

class ArcsineModel final : public UnaryTrigModel
{
public:
  QString caption() const override { return QStringLiteral("arcsin"); }

  QString name() const override { return QStringLiteral("arcsin"); }

  ir::UnaryTrigExpr::ID GetID() const override
  {
    return ir::UnaryTrigExpr::ID::Arcsine;
  }
};

class ArccosineModel final : public UnaryTrigModel
{
public:
  QString caption() const override { return QStringLiteral("arccos"); }

  QString name() const override { return QStringLiteral("arccos"); }

  ir::UnaryTrigExpr::ID GetID() const override
  {
    return ir::UnaryTrigExpr::ID::Arccosine;
  }
};

class ArctangentModel final : public UnaryTrigModel
{
public:
  QString caption() const override { return QStringLiteral("arctan"); }

  QString name() const override { return QStringLiteral("arctan"); }

  ir::UnaryTrigExpr::ID GetID() const override
  {
    return ir::UnaryTrigExpr::ID::Arctangent;
  }
};

} // namespace

void
DefineTrigModels(QtNodes::DataModelRegistry& registry)
{
  registry.registerModel<SineModel>("Trigonometry");
  registry.registerModel<CosineModel>("Trigonometry");
  registry.registerModel<TangentModel>("Trigonometry");

  registry.registerModel<ArcsineModel>("Trigonometry");
  registry.registerModel<ArccosineModel>("Trigonometry");
  registry.registerModel<ArctangentModel>("Trigonometry");
}
