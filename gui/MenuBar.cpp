#include "MenuBar.h"

#include "MenuBarObserver.h"

//#include <QFileDialog>
#include <QMenuBar>

namespace {

class SignalProxy : public QObject
{
  Q_OBJECT
public:
  using Observer = gui::MenuBarObserver;

  void AddObserver(std::unique_ptr<Observer> observer)
  {
    mObservers.emplace_back(std::move(observer));
  }

public slots:
  void OnSave()
  {
    for (auto& observer : mObservers)
      observer->ObserveSave();
#if 0
    QFileDialog dialog(nullptr, tr("Save Project"));

    dialog.setAcceptMode(QFileDialog::AcceptSave);

    dialog.setNameFilter("*.json");

    dialog.exec();
#endif
  }

private:
  std::vector<std::unique_ptr<Observer>> mObservers;
};

class MenuBarImpl final : public gui::MenuBar
{
public:
  MenuBarImpl(QWidget* parent)
    : mMenuBar(parent)
  {
    MakeFileMenu();
  }

  QMenuBar* GetMenuBar() override { return &mMenuBar; }

  void AddObserver(std::unique_ptr<gui::MenuBarObserver> observer) override
  {
    mSignalProxy.AddObserver(std::move(observer));
  }

private:
  void MakeFileMenu()
  {
    auto* fileMenu = mMenuBar.addMenu(QObject::tr("&File"));

    auto* saveAction = fileMenu->addAction(QObject::tr("Save"));
    saveAction->setShortcuts(QKeySequence::Save);
    saveAction->setStatusTip(QObject::tr("Save the project changes to file."));

    QObject::connect(
      saveAction, &QAction::triggered, &mSignalProxy, &SignalProxy::OnSave);
  }

private:
  QMenuBar mMenuBar;

  SignalProxy mSignalProxy;
};

} // namespace

namespace gui {

auto
MenuBar::Make(QWidget* parent) -> std::unique_ptr<MenuBar>
{
  return std::unique_ptr<MenuBar>(new MenuBarImpl(parent));
}

} // namespace gui

#include "MenuBar.moc"
