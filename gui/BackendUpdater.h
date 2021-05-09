#pragma once

#include <memory>

class Backend;
class ProjectObserver;

auto
MakeBackendUpdater(std::shared_ptr<Backend> backend)
  -> std::unique_ptr<ProjectObserver>;
