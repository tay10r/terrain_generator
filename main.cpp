#include <QApplication>
#include <QHBoxLayout>
#include <QMainWindow>
#include <QOpenGLWidget>
#include <QPushButton>
#include <QSplitter>
#include <QToolBar>
#include <QWidget>

#include <nodes/ConnectionStyle>
#include <nodes/FlowScene>
#include <nodes/FlowView>
#include <nodes/NodeData>
#include <nodes/TypeConverter>

#include <nodes/NodeDataModel>

#include <iostream>
#include <memory>

#include <limits.h>

#include <GL/gl.h>

#include "Backend.h"
#include "IR.h"

namespace {

class BackendObject;

using UniqueExprPtr = std::unique_ptr<ir::Expr>;

class ExprNodeData final : public QtNodes::NodeData
{
public:
  ExprNodeData(UniqueExprPtr&& expr)
    : mExpr(std::move(expr))
  {}

  QtNodes::NodeDataType type() const override
  {
    auto exprType = mExpr->GetType();

    if (!exprType)
      return QtNodes::NodeDataType{ "", "" };

    switch (*exprType) {
      case ir::Type::Float:
        return QtNodes::NodeDataType{ "float", "" };
    }

    return QtNodes::NodeDataType{ "", "" };
  }

  const ir::Expr& GetExpr() const noexcept { return *mExpr; }

private:
  UniqueExprPtr mExpr;
};

auto
MakeNodeData(ir::Expr* expr) -> std::shared_ptr<QtNodes::NodeData>
{
  using RetType = std::shared_ptr<QtNodes::NodeData>;

  return RetType(new ExprNodeData(UniqueExprPtr(expr)));
}

class CoordinatesModel : public QtNodes::NodeDataModel
{
public:
  QString caption() const override { return QStringLiteral("Coordinates"); }

  QString name() const override { return QStringLiteral("Coordinates"); }

  QJsonObject save() const override { return QJsonObject(); }

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
        return QtNodes::NodeDataType{ "float", "x" };
      case 1:
        return QtNodes::NodeDataType{ "float", "y" };
    }
    return QtNodes::NodeDataType{ "", "" };
  }

  auto outData(QtNodes::PortIndex portIndex)
    -> std::shared_ptr<QtNodes::NodeData> override
  {
    switch (portIndex) {
      case 0:
        return MakeNodeData(new ir::VarRefExpr(ir::VarRefExpr::ID::XCoord));
      case 1:
        return MakeNodeData(new ir::VarRefExpr(ir::VarRefExpr::ID::YCoord));
    }
    return nullptr;
  }

  void setInData(std::shared_ptr<QtNodes::NodeData>, int) override {}

  QWidget* embeddedWidget() override { return nullptr; }
};

class VertexOutputModel : public QtNodes::NodeDataModel
{
public:
  VertexOutputModel(std::shared_ptr<BackendObject> backendObject)
    : mBackendObject(backendObject)
  {}

  QString caption() const override { return QStringLiteral("Vertex Output"); }

  QString name() const override { return QStringLiteral("Vertex Output"); }

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
        return QtNodes::NodeDataType{ "float", "height" };
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
    std::cout << "set in" << std::endl;

    switch (portIndex) {
      case 0:
        UpdateVertexOutput(*nodeData);
        break;
    }
  }

  QWidget* embeddedWidget() override { return nullptr; }

private:
  void UpdateVertexOutput(const QtNodes::NodeData& nodeData);

  std::shared_ptr<BackendObject> mBackendObject;
};

class Project
{
public:
  static constexpr int GetDefaultWidth() { return 1024; }

  static constexpr int GetDefaultHeight() { return 1024; }

  Project()
  {
    QJsonObject resolution;
    resolution["width"] = QJsonValue(GetDefaultWidth());
    resolution["height"] = QJsonValue(GetDefaultHeight());
    mRoot["resolution"] = resolution;
  }

  int GetResWidth() const
  {
    return mRoot["resolution"]["width"].toInt(GetDefaultWidth());
  }

  int GetResHeight() const
  {
    return mRoot["resolution"]["height"].toInt(GetDefaultHeight());
  }

private:
  QJsonObject mRoot;
};

class BackendObject final : public QObject
{
  Q_OBJECT
public:
  BackendObject(std::unique_ptr<Backend> backend)
    : mBackend(std::move(backend))
  {}

  void OpenProject(const Project& project)
  {
    auto w = project.GetResWidth();
    auto h = project.GetResHeight();

    w = (w <= 0) ? Project::GetDefaultWidth() : w;

    h = (h <= 0) ? Project::GetDefaultHeight() : h;

    mBackend->Resize(size_t(w), size_t(h));
  }

  void Display() { mBackend->Display(); }

  void UpdateHeightMapExpr(const ir::Expr& expr)
  {
    auto success = mBackend->UpdateHeightMapExpr(expr);

    if (!success) {
      // TODO : log error
    }

    mBackend->ComputeHeightMap();

    emit HeightMapChanged();
  }

  // clang-format off
signals:
  // clang-format on
  void HeightMapChanged();

private:
  std::unique_ptr<Backend> mBackend;
};

class RenderWidget : public QOpenGLWidget
{
public:
  RenderWidget(std::shared_ptr<BackendObject> backendObject)
    : mBackendObject(backendObject)
  {
    setMinimumSize(320, 240);
  }

protected:
  void paintGL() override { mBackendObject->Display(); }

  void resizeGL(int w, int h) override { glViewport(0, 0, w, h); }

  void initializeGL() override { glClearColor(0.0f, 0.0f, 0.0f, 1.0f); }

private:
  std::shared_ptr<BackendObject> mBackendObject;
};

class ToolBar : public QToolBar
{
public:
  ToolBar(QWidget* parent)
    : QToolBar(parent)
    , mResizeButton("Resize", this)
  {
    addWidget(&mResizeButton);
  }

private:
  QPushButton mResizeButton;
};

class Workspace : public QSplitter
{
  Q_OBJECT
public:
  Workspace(std::shared_ptr<BackendObject> backendObject, QWidget* parent)
    : QSplitter(Qt::Horizontal, parent)
    , mBackendObject(backendObject)
  {
    std::shared_ptr<QtNodes::DataModelRegistry> dataModels(
      new QtNodes::DataModelRegistry());

    dataModels->registerModel<CoordinatesModel>("Inputs");

    // dataModels->registerModel<VertexOutputModel>("Outputs");

    mFlowScene = new QtNodes::FlowScene(dataModels, this);

    mFlowScene->createNode(std::unique_ptr<QtNodes::NodeDataModel>(
      new VertexOutputModel(backendObject)));

    mFlowView = new QtNodes::FlowView(mFlowScene);

    mRenderWidget = new RenderWidget(backendObject);

    addWidget(mFlowView);

    addWidget(mRenderWidget);

    setSizes(QList<int>({ INT_MAX, INT_MAX }));
  }

  void OpenProject(const Project& project)
  {
    mBackendObject->OpenProject(project);
  }

public slots:
  void QueueRender() { mRenderWidget->update(); }

private:
  QtNodes::FlowScene* mFlowScene = nullptr;

  QtNodes::FlowView* mFlowView = nullptr;

  RenderWidget* mRenderWidget = nullptr;

  std::shared_ptr<BackendObject> mBackendObject;
};

} // namespace

int
main(int argc, char** argv)
{
  Project project;

  auto backendObject =
    std::make_shared<BackendObject>(Backend::MakeCpuBackend());

  QApplication app(argc, argv);

  QMainWindow mainWindow;

  mainWindow.setWindowTitle("Terrain Generator");

  mainWindow.showMaximized();

  QWidget mainWidget;

  mainWindow.setCentralWidget(&mainWidget);

  QVBoxLayout layout(&mainWidget);

  mainWidget.setLayout(&layout);

  ToolBar toolBar(&mainWidget);

  Workspace workspace(backendObject, &mainWidget);

  layout.addWidget(&toolBar);

  layout.addWidget(&workspace);

  workspace.OpenProject(project);

  QObject::connect(backendObject.get(),
                   &BackendObject::HeightMapChanged,
                   &workspace,
                   &Workspace::QueueRender);

  return app.exec();
}

namespace {

void
VertexOutputModel::UpdateVertexOutput(const QtNodes::NodeData& nodeData)
{
  auto* exprNodeData = dynamic_cast<const ExprNodeData*>(&nodeData);

  if (!exprNodeData)
    return;

  mBackendObject->UpdateHeightMapExpr(exprNodeData->GetExpr());
}

} // namespace

#include "main.moc"
