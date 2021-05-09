#pragma once

#include <memory>

class QWidget;
class QMenuBar;

namespace gui {

class MenuBarObserver;

class MenuBar
{
public:
  static auto Make(QWidget* parent) -> std::unique_ptr<MenuBar>;

  virtual ~MenuBar() = default;

  virtual void AddObserver(std::unique_ptr<MenuBarObserver>) = 0;

  virtual QMenuBar* GetMenuBar() = 0;
};

} // namespace gui
