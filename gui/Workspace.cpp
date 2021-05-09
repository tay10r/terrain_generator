#include "gui/Workspace.h"

#include <QSplitter>

namespace {

class WorkspaceImpl final : public Workspace
{
public:
  WorkspaceImpl(QWidget* parent)
    : mSplitter(parent)
  {}

  void AddWidget(QWidget* widget) override
  {
    mSplitter.addWidget(widget);

    UpdateSizes();
  }

  QWidget* GetWidget() override { return &mSplitter; }

private:
  void UpdateSizes()
  {
    int minSize = 1;

    for (int i = 0; i < mSplitter.count(); i++) {

      auto minWidgetWidth = mSplitter.widget(i)->minimumSizeHint().width();

      minSize = std::max(minSize, minWidgetWidth);
    }

    QList<int> sizes;

    for (int i = 0; i < mSplitter.count(); i++)
      sizes.push_back(minSize);

    mSplitter.setSizes(sizes);
  }

private:
  QSplitter mSplitter;
};

} // namespace

auto
Workspace::Make(QWidget* parent) -> std::unique_ptr<Workspace>
{
  return std::unique_ptr<Workspace>(new WorkspaceImpl(parent));
}
