#pragma once

#include <memory>

class QWidget;
class HeightMapObserver;

class SceneView
{
public:
  static auto Make(QWidget* parent) -> std::shared_ptr<SceneView>;

  virtual ~SceneView() = default;

  virtual QWidget* GetWidget() = 0;

  virtual auto MakeHeightMapUpdater() -> std::unique_ptr<HeightMapObserver> = 0;
};
