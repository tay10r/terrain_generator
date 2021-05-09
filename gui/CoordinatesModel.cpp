#include "CoordinatesModel.h"

#include "gui/ExprNodeData.h"

#include <nodes/DataModelRegistry>

#include "core/IR.h"

namespace {

class CoordinatesModel : public QtNodes::NodeDataModel
{
public:
  QString caption() const override { return QStringLiteral("Coordinates"); }

  QString name() const override { return QStringLiteral("Coordinates"); }

  void restore(const QJsonObject&) override {}

  unsigned int nPorts(QtNodes::PortType portType) const override
  {
    switch (portType) {
      case QtNodes::PortType::None:
      case QtNodes::PortType::In:
        break;
      case QtNodes::PortType::Out:
        return 2;
    }

    return 0;
  }

  auto dataType(QtNodes::PortType, QtNodes::PortIndex portIndex) const
    -> QtNodes::NodeDataType override
  {
    switch (portIndex) {
      case 0:
        return QtNodes::NodeDataType{ "float", "Center U" };
      case 1:
        return QtNodes::NodeDataType{ "float", "Center V" };
    }
    return QtNodes::NodeDataType{ "", "" };
  }

  auto outData(QtNodes::PortIndex portIndex)
    -> std::shared_ptr<QtNodes::NodeData> override
  {
    switch (portIndex) {
      case 0:
        return ExprToNodeData(
          new ir::VarRefExpr(ir::VarRefExpr::ID::CenterUCoord));
      case 1:
        return ExprToNodeData(
          new ir::VarRefExpr(ir::VarRefExpr::ID::CenterVCoord));
    }
    return nullptr;
  }

  void setInData(std::shared_ptr<QtNodes::NodeData>, int) override {}

  QWidget* embeddedWidget() override { return nullptr; }
};

} // namespace

void
DefineCoordinatesModel(QtNodes::DataModelRegistry& registry)
{
  registry.registerModel<CoordinatesModel>("Inputs");
}
