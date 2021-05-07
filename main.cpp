#include <QApplication>
#include <QHBoxLayout>
#include <QMainWindow>
#include <QOpenGLWidget>
#include <QPushButton>
#include <QSpinBox>
#include <QSplitter>
#include <QTimer>
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

using SharedNodeDataPtr = std::shared_ptr<QtNodes::NodeData>;

QtNodes::NodeDataType
ToNodeDataType(ir::Type type, const char* name = "")
{
  switch (type) {
    case ir::Type::Int:
      return QtNodes::NodeDataType{ "int", name };
    case ir::Type::Float:
      return QtNodes::NodeDataType{ "float", name };
  }
  return QtNodes::NodeDataType{ "", "" };
}

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
    else
      return ToNodeDataType(*exprType);
  }

  const ir::Expr* GetExpr() const noexcept { return mExpr.get(); }

private:
  UniqueExprPtr mExpr;
};

auto
MakeNodeData(ir::Expr* expr) -> SharedNodeDataPtr
{
  return SharedNodeDataPtr(new ExprNodeData(UniqueExprPtr(expr)));
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
        return MakeNodeData(
          new ir::VarRefExpr(ir::VarRefExpr::ID::CenterUCoord));
      case 1:
        return MakeNodeData(
          new ir::VarRefExpr(ir::VarRefExpr::ID::CenterVCoord));
    }
    return nullptr;
  }

  void setInData(std::shared_ptr<QtNodes::NodeData>, int) override {}

  QWidget* embeddedWidget() override { return nullptr; }
};

class VertexOutputModel : public QtNodes::NodeDataModel
{
public:
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
        mNodeData = nodeData;
        break;
    }
  }

  QWidget* embeddedWidget() override { return nullptr; }

  auto ToIRExpr() -> const ir::Expr*
  {
    if (!mNodeData)
      return nullptr;

    const auto* derived = dynamic_cast<const ExprNodeData*>(mNodeData.get());
    if (!derived)
      return nullptr;

    return derived->GetExpr();
  }

private:
  SharedNodeDataPtr mNodeData;
};

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

private:
  std::shared_ptr<BackendObject> mBackendObject;
};

class FloatLiteralModel final : public LiteralModel
{
  Q_OBJECT
public:
  FloatLiteralModel()
  {
    connect(&mSpinBox,
            QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            [this](double) { OnDataChange(); });
  }

  const char* GetName() const noexcept { return "Float Constant"; }

  QAbstractSpinBox* GetSpinBox() override { return &mSpinBox; }

  auto outData(QtNodes::PortIndex) -> SharedNodeDataPtr override
  {
    return MakeNodeData(new ir::LiteralExpr<float>(mSpinBox.value()));
  }

  auto dataType(QtNodes::PortType, QtNodes::PortIndex) const
    -> QtNodes::NodeDataType override
  {
    return QtNodes::NodeDataType{ "float", "Value" };
  }

protected slots:
  void OnDataChange() { emit dataUpdated(0); }

private:
  QDoubleSpinBox mSpinBox;
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

  void Render()
  {
    mBackend->ComputeHeightMap();

    mBackend->Display();
  }

  void UpdateHeightMapExpr(const ir::Expr& expr)
  {
    auto success = mBackend->UpdateHeightMapExpr(expr);

    if (!success) {
      // TODO : log error
    }
  }

private:
  std::unique_ptr<Backend> mBackend;
};

class RenderWidget : public QOpenGLWidget
{
public:
  RenderWidget(std::shared_ptr<BackendObject> backendObject,
               VertexOutputModel* vertexOutputModel)
    : mBackendObject(backendObject)
    , mVertexOutputModel(vertexOutputModel)
    , mTimer(new QTimer(this))
  {
    setMinimumSize(320, 240);

    connect(mTimer, &QTimer::timeout, [this]() { update(); });

    mTimer->start(1000 / 24);
  }

protected:
  void paintGL() override
  {
    const auto* irExpr = mVertexOutputModel->ToIRExpr();

    if (irExpr) {
      mBackendObject->UpdateHeightMapExpr(*irExpr);
    }

    mBackendObject->Render();
  }

  void resizeGL(int w, int h) override { glViewport(0, 0, w, h); }

  void initializeGL() override { glClearColor(0.0f, 0.0f, 0.0f, 1.0f); }

private:
  std::shared_ptr<BackendObject> mBackendObject;

  VertexOutputModel* mVertexOutputModel = nullptr;

  QTimer* mTimer = nullptr;
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
    // dataModels->registerModel<LiteralModel<int>>("Inputs");
    dataModels->registerModel<FloatLiteralModel>("Inputs");

    mFlowScene = new QtNodes::FlowScene(dataModels, this);

    auto vertexOutputModel = std::make_unique<VertexOutputModel>();

    mVertexOutputModel = vertexOutputModel.get();

    mFlowScene->createNode(std::move(vertexOutputModel));

    mFlowView = new QtNodes::FlowView(mFlowScene);

    mRenderWidget = new RenderWidget(backendObject, mVertexOutputModel);

    addWidget(mFlowView);

    addWidget(mRenderWidget);

    setSizes(QList<int>({ INT_MAX, INT_MAX }));
  }

  void OpenProject(const Project& project)
  {
    mBackendObject->OpenProject(project);
  }

private:
  QtNodes::FlowScene* mFlowScene = nullptr;

  QtNodes::FlowView* mFlowView = nullptr;

  RenderWidget* mRenderWidget = nullptr;

  VertexOutputModel* mVertexOutputModel = nullptr;

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

  return app.exec();
}

#include "main.moc"
