#include "gui/BackendUpdater.h"

#include "gui/ProjectObserver.h"

#include "core/Backend.h"

namespace {

class BackendUpdater final : public ProjectObserver
{
public:
  BackendUpdater(std::shared_ptr<Backend> backend)
    : mBackend(backend)
  {}

  void ObserveHeightChange(const ir::Expr* heightExpr) override
  {
    mBackend->UpdateHeightExpr(heightExpr);

    mBackend->ComputeHeightMap();
  }

  void ObserveSurfaceChange(const ir::Expr* colorExpr) override
  {
    mBackend->UpdateColorExpr(colorExpr);

    mBackend->ComputeSurface();
  }

private:
  std::shared_ptr<Backend> mBackend;
};

} // namespace

auto
MakeBackendUpdater(std::shared_ptr<Backend> backend)
  -> std::unique_ptr<ProjectObserver>
{
  return std::unique_ptr<ProjectObserver>(new BackendUpdater(backend));
}
