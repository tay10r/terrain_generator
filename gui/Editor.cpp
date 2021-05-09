#include "gui/Editor.h"

#include "gui/ConstantsModels.h"
#include "gui/CoordinatesModel.h"
#include "gui/MenuBarObserver.h"
#include "gui/OutputModels.h"
#include "gui/ProjectObserver.h"

#include <QCheckBox>
#include <QFile>
#include <QFormLayout>
#include <QJsonDocument>
#include <QSpinBox>
#include <QTabWidget>

#include <vector>

#include <nodes/DataModelRegistry>
#include <nodes/FlowScene>
#include <nodes/FlowView>

namespace {

using namespace QtNodes;

class CompoundProjectObserver final : public ProjectObserver
{
public:
  using Observer = ProjectObserver;

  void AddObserver(std::unique_ptr<Observer> observer)
  {
    mObservers.emplace_back(std::move(observer));
  }

  void ObserveSurfaceChange(const ir::Expr* colorExpr) override
  {
    for (auto& observer : mObservers)
      observer->ObserveSurfaceChange(colorExpr);
  }

  void ObserveHeightChange(const ir::Expr* heightExpr) override
  {
    for (auto& observer : mObservers)
      observer->ObserveHeightChange(heightExpr);
  }

private:
  std::vector<std::unique_ptr<Observer>> mObservers;
};

class PropertiesEditor final
{
public:
  PropertiesEditor(QWidget* parent)
    : mWidget(parent)
    , mLayout(&mWidget)
    , mWidthBox(&mWidget)
    , mHeightBox(&mWidget)
    , mSizeLock(&mWidget)
  {
    mLayout.addRow("Width", &mWidthBox);

    mLayout.addRow("Height", &mHeightBox);

    mLayout.addRow("Size Locked", &mSizeLock);

    mLayout.setFieldGrowthPolicy(QFormLayout::FieldsStayAtSizeHint);

    mWidthBox.setMinimum(2);
    mWidthBox.setMaximum(32768);

    mHeightBox.setMinimum(2);
    mHeightBox.setMaximum(32768);
  }

  QWidget* GetWidget() { return &mWidget; }

  QJsonObject ToJson() const
  {
    QJsonObject jsonObj;
    jsonObj["terrain_width"] = mWidthBox.value();
    jsonObj["terrain_height"] = mHeightBox.value();
    jsonObj["terrain_size_lock"] = mSizeLock.isChecked();
    return jsonObj;
  }

private:
  QWidget mWidget;

  QFormLayout mLayout;

  QSpinBox mWidthBox;

  QSpinBox mHeightBox;

  QCheckBox mSizeLock;
};

class HeightEditor final
{
public:
  HeightEditor(QWidget* parent, std::shared_ptr<ProjectObserver> observer)
    : mDataModelRegistry(MakeDataModelRegistry())
    , mFlowScene(mDataModelRegistry, parent)
    , mFlowView(&mFlowScene)
  {
    mFlowScene.createNode(MakeHeightModel(observer));
  }

  QWidget* GetWidget() { return &mFlowView; }

  QJsonObject ToJson() const
  {
    return mFlowScene.saveToObject();
  }

private:
  static auto MakeDataModelRegistry() -> std::shared_ptr<DataModelRegistry>
  {
    auto registry = std::make_shared<DataModelRegistry>();

    DefineConstantsModels(*registry);

    DefineCoordinatesModel(*registry);

    return registry;
  }

private:
  std::shared_ptr<DataModelRegistry> mDataModelRegistry;

  FlowScene mFlowScene;

  FlowView mFlowView;
};

class SurfaceEditor final
{
public:
  SurfaceEditor(QWidget* parent, std::shared_ptr<ProjectObserver> observer)
    : mDataModelRegistry(MakeDataModelRegistry())
    , mFlowScene(mDataModelRegistry, parent)
    , mFlowView(&mFlowScene)
  {
    mFlowScene.createNode(MakeSurfaceModel(observer));
  }

  QWidget* GetWidget() { return &mFlowView; }

  QJsonObject ToJson() const
  {
    return mFlowScene.saveToObject();
  }

private:
  static auto MakeDataModelRegistry() -> std::shared_ptr<DataModelRegistry>
  {
    auto registry = std::make_shared<DataModelRegistry>();

    DefineConstantsModels(*registry);

    DefineCoordinatesModel(*registry);

    return registry;
  }

private:
  std::shared_ptr<DataModelRegistry> mDataModelRegistry;

  FlowScene mFlowScene;

  FlowView mFlowView;
};

class MenuBarProxy final : public gui::MenuBarObserver
{
public:
  MenuBarProxy(gui::Editor* editor)
    : mEditor(editor)
  {}

  void ObserveSave() override { mEditor->Save(); }

private:
  gui::Editor* mEditor;
};

class EditorImpl final : public gui::Editor
{
public:
  EditorImpl(QWidget* parent)
    : mTabWidget(parent)
    , mCompoundObserver(new CompoundProjectObserver())
    , mHeightEditor(&mTabWidget, mCompoundObserver)
    , mSurfaceEditor(&mTabWidget, mCompoundObserver)
    , mPropertiesEditor(&mTabWidget)
  {
    mTabWidget.setMinimumSize(320, 240);

    mTabWidget.addTab(mHeightEditor.GetWidget(), QObject::tr("Height"));

    mTabWidget.addTab(mSurfaceEditor.GetWidget(), QObject::tr("Surface"));

    mTabWidget.addTab(mPropertiesEditor.GetWidget(), QObject::tr("Properties"));
  }

  void AddProjectObserver(std::unique_ptr<ProjectObserver> observer) override
  {
    auto* o = dynamic_cast<CompoundProjectObserver*>(mCompoundObserver.get());
    if (o)
      o->AddObserver(std::move(observer));
  }

  QWidget* GetWidget() override { return &mTabWidget; }

  bool Save() const override { return Save("terrain.json"); }

  bool Save(const char* path) const override
  {
    QJsonObject root;

    root["properties"] = mPropertiesEditor.ToJson();

    root["height"] = mHeightEditor.ToJson();

    root["surface"] = mSurfaceEditor.ToJson();

    QJsonDocument jsonDoc;

    jsonDoc.setObject(root);

    QFile file(path);

    if (!file.open(QFile::WriteOnly | QFile::Text | QFile::Truncate))
      return false;

    file.write(jsonDoc.toJson());

    return true;
  }

  auto MakeMenuBarObserver() -> std::unique_ptr<gui::MenuBarObserver> override
  {
    return std::unique_ptr<gui::MenuBarObserver>(new MenuBarProxy(this));
  }

private:
  QTabWidget mTabWidget;

  std::shared_ptr<ProjectObserver> mCompoundObserver;

  HeightEditor mHeightEditor;

  SurfaceEditor mSurfaceEditor;

  PropertiesEditor mPropertiesEditor;
};

} // namespace

namespace gui {

auto
Editor::Make(QWidget* parent) -> std::unique_ptr<Editor>
{
  return std::unique_ptr<Editor>(new EditorImpl(parent));
}

} // namespace gui