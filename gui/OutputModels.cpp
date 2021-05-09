#include "gui/OutputModels.h"

#include "gui/ExprNodeData.h"
#include "gui/ProjectObserver.h"

#include <nodes/NodeDataModel>

namespace {

class HeightModel final : public QtNodes::NodeDataModel
{
public:
  HeightModel(std::shared_ptr<ProjectObserver> observer)
    : mObserver(observer)
  {}

  QString caption() const override { return QStringLiteral("Output"); }

  QString name() const override { return QStringLiteral("Output"); }

  QJsonObject save() const override { return QJsonObject(); }

  void restore(const QJsonObject&) override {}

  unsigned int nPorts(QtNodes::PortType portType) const override
  {
    switch (portType) {
      case QtNodes::PortType::None:
      case QtNodes::PortType::In:
        return 1;
      case QtNodes::PortType::Out:
        break;
    }

    return 0;
  }

  auto dataType(QtNodes::PortType, QtNodes::PortIndex portIndex) const
    -> QtNodes::NodeDataType override
  {
    switch (portIndex) {
      case 0:
        return QtNodes::NodeDataType{ "float", "Height" };
    }

    return QtNodes::NodeDataType{ "", "" };
  }

  auto outData(QtNodes::PortIndex)
    -> std::shared_ptr<QtNodes::NodeData> override
  {
    return nullptr;
  }

  void setInData(std::shared_ptr<QtNodes::NodeData> nodeData,
                 QtNodes::PortIndex portIndex) override
  {
    switch (portIndex) {
      case 0:
        mHeightData = nodeData;
        break;
    }

    NotifyObservers();
  }

  QWidget* embeddedWidget() override { return nullptr; }

private:
  void NotifyObservers()
  {
    const ir::Expr* heightExpr = nullptr;

    if (mHeightData)
      heightExpr = NodeDataToExpr(mHeightData.get());

    mObserver->ObserveHeightChange(heightExpr);
  }

private:
  std::shared_ptr<QtNodes::NodeData> mHeightData;

  std::shared_ptr<ProjectObserver> mObserver;
};

class SurfaceModel final : public QtNodes::NodeDataModel
{
public:
  SurfaceModel(std::shared_ptr<ProjectObserver> observer)
    : mObserver(observer)
  {}

  QString caption() const override { return QStringLiteral("Surface"); }

  QString name() const override { return QStringLiteral("Surface"); }

  QJsonObject save() const override { return QJsonObject(); }

  void restore(const QJsonObject&) override {}

  unsigned int nPorts(QtNodes::PortType portType) const override
  {
    switch (portType) {
      case QtNodes::PortType::None:
      case QtNodes::PortType::In:
        return 1;
      case QtNodes::PortType::Out:
        break;
    }

    return 0;
  }

  auto dataType(QtNodes::PortType, QtNodes::PortIndex portIndex) const
    -> QtNodes::NodeDataType override
  {
    switch (portIndex) {
      case 0:
        return QtNodes::NodeDataType{ "vec3", "Color" };
    }

    return QtNodes::NodeDataType{ "", "" };
  }

  auto outData(QtNodes::PortIndex)
    -> std::shared_ptr<QtNodes::NodeData> override
  {
    return nullptr;
  }

  void setInData(std::shared_ptr<QtNodes::NodeData> nodeData,
                 QtNodes::PortIndex portIndex) override
  {
    switch (portIndex) {
      case 0:
        mColorData = nodeData;
        break;
    }

    NotifyObservers();
  }

  QWidget* embeddedWidget() override { return nullptr; }

private:
  void NotifyObservers()
  {
    const ir::Expr* colorExpr = nullptr;

    if (mColorData)
      colorExpr = NodeDataToExpr(mColorData.get());

    mObserver->ObserveSurfaceChange(colorExpr);
  }

private:
  std::shared_ptr<QtNodes::NodeData> mColorData;

  std::shared_ptr<ProjectObserver> mObserver;
};

} // namespace

auto
MakeHeightModel(std::shared_ptr<ProjectObserver> observer)
  -> std::unique_ptr<QtNodes::NodeDataModel>
{
  return std::unique_ptr<QtNodes::NodeDataModel>(new HeightModel(observer));
}

auto
MakeSurfaceModel(std::shared_ptr<ProjectObserver> observer)
  -> std::unique_ptr<QtNodes::NodeDataModel>
{
  return std::unique_ptr<QtNodes::NodeDataModel>(new SurfaceModel(observer));
}
