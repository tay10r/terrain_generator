#include "ConstantsModels.h"

#include "core/IR.h"

#include "gui/ExprNodeData.h"

#include <nodes/DataModelRegistry>
#include <nodes/NodeDataModel>

#include <QDoubleSpinBox>
#include <QSpinBox>

namespace {

class LiteralModel : public QtNodes::NodeDataModel
{
public:
  virtual const char* GetName() const noexcept = 0;

  QString caption() const override { return GetName(); }

  QString name() const override { return GetName(); }

  QJsonObject save() const override { return QJsonObject(); }

  void restore(const QJsonObject&) override {}

  unsigned int nPorts(QtNodes::PortType portType) const override
  {
    switch (portType) {
      case QtNodes::PortType::None:
      case QtNodes::PortType::In:
        break;
      case QtNodes::PortType::Out:
        return 1;
    }

    return 0;
  }

  void setInData(std::shared_ptr<QtNodes::NodeData>,
                 QtNodes::PortIndex) override
  {}

  QWidget* embeddedWidget() override { return GetSpinBox(); }

  virtual QAbstractSpinBox* GetSpinBox() = 0;
};

class FloatLiteralModel final : public LiteralModel
{
  Q_OBJECT
public:
  FloatLiteralModel()
    : mSpinBox(new QDoubleSpinBox())
  {
    connect(mSpinBox,
            QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            [this](double) { OnDataChange(); });
  }

  const char* GetName() const noexcept override { return "Float Constant"; }

  QAbstractSpinBox* GetSpinBox() override { return mSpinBox; }

  auto outData(QtNodes::PortIndex)
    -> std::shared_ptr<QtNodes::NodeData> override
  {
    return ExprToNodeData(new ir::LiteralExpr<float>(mSpinBox->value()));
  }

  auto dataType(QtNodes::PortType, QtNodes::PortIndex) const
    -> QtNodes::NodeDataType override
  {
    return QtNodes::NodeDataType{ "float", "Value" };
  }

protected slots:
  void OnDataChange() { emit dataUpdated(0); }

private:
  QDoubleSpinBox* mSpinBox;
};

class IntLiteralModel final : public LiteralModel
{
  Q_OBJECT
public:
  IntLiteralModel()
    : mSpinBox(new QSpinBox())
  {
    connect(mSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), [this](int) {
      OnDataChange();
    });
  }

  const char* GetName() const noexcept override { return "Integer Constant"; }

  QAbstractSpinBox* GetSpinBox() override { return mSpinBox; }

  auto outData(QtNodes::PortIndex)
    -> std::shared_ptr<QtNodes::NodeData> override
  {
    return ExprToNodeData(new ir::LiteralExpr<int>(mSpinBox->value()));
  }

  auto dataType(QtNodes::PortType, QtNodes::PortIndex) const
    -> QtNodes::NodeDataType override
  {
    return QtNodes::NodeDataType{ "int", "Value" };
  }

protected slots:
  void OnDataChange() { emit dataUpdated(0); }

private:
  QSpinBox* mSpinBox;
};

} // namespace

void
DefineConstantsModels(QtNodes::DataModelRegistry& registry)
{
  registry.registerModel<FloatLiteralModel>("Inputs");

  registry.registerModel<IntLiteralModel>("Inputs");
}

#include "ConstantsModels.moc"
