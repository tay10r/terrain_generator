#pragma once

#include <memory>

namespace QtNodes {

class NodeDataModel;

} // namespace QtNodes

class ProjectObserver;

auto MakeHeightModel(std::shared_ptr<ProjectObserver>)
  -> std::unique_ptr<QtNodes::NodeDataModel>;

auto MakeSurfaceModel(std::shared_ptr<ProjectObserver>)
  -> std::unique_ptr<QtNodes::NodeDataModel>;
