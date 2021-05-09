#pragma once

namespace gui {

class MenuBarObserver
{
public:
  virtual ~MenuBarObserver() = default;

  virtual void ObserveSave() = 0;
};

} // namespace gui
