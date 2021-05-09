#include <QApplication>
#include <QMainWindow>
#include <QVBoxLayout>

#include "core/Backend.h"
#include "core/HeightMapObserver.h"

#include "gui/BackendUpdater.h"
#include "gui/Editor.h"
#include "gui/MenuBar.h"
#include "gui/MenuBarObserver.h"
#include "gui/ProjectObserver.h"
#include "gui/SceneView.h"
#include "gui/Workspace.h"

int
main(int argc, char** argv)
{
  QApplication app(argc, argv);

  QMainWindow mainWindow;

  mainWindow.setWindowTitle("Terrain Generator");

  mainWindow.showMaximized();

  auto menuBar = gui::MenuBar::Make(&mainWindow);

  mainWindow.setMenuBar(menuBar->GetMenuBar());

  QWidget mainWidget;

  mainWindow.setCentralWidget(&mainWidget);

  QVBoxLayout layout(&mainWidget);

  mainWidget.setLayout(&layout);

  std::shared_ptr<Backend> backend;

  backend.reset(Backend::MakeCpuBackend().release());

  backend->Resize(2, 2);

  auto workspace = Workspace::Make(&mainWidget);

  auto sceneView = SceneView::Make(workspace->GetWidget());

  auto editor = gui::Editor::Make(workspace->GetWidget());

  editor->AddProjectObserver(MakeBackendUpdater(backend));

  menuBar->AddObserver(editor->MakeMenuBarObserver());

  backend->AddHeightMapObserver(sceneView->MakeHeightMapUpdater());

  workspace->AddWidget(editor->GetWidget());

  workspace->AddWidget(sceneView->GetWidget());

  layout.addWidget(workspace->GetWidget());

  return app.exec();
}
