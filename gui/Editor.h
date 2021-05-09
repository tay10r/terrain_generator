#pragma once

#include <memory>

class QWidget;
class ProjectObserver;

namespace gui {

class MenuBarObserver;

class Editor
{
public:
  static auto Make(QWidget* parent) -> std::unique_ptr<Editor>;

  virtual ~Editor() = default;

  virtual void AddProjectObserver(std::unique_ptr<ProjectObserver>) = 0;

  virtual auto MakeMenuBarObserver() -> std::unique_ptr<MenuBarObserver> = 0;

  virtual QWidget* GetWidget() = 0;

  virtual bool Save() const = 0;

  virtual bool Save(const char* path) const = 0;
};

} // namespace gui
