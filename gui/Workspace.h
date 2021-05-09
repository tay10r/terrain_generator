#pragma once

#include <memory>

class QWidget;

class Workspace
{
public:
  static auto Make(QWidget* parent) -> std::unique_ptr<Workspace>;

  virtual ~Workspace() = default;

  virtual void AddWidget(QWidget*) = 0;

  virtual QWidget* GetWidget() = 0;
};
