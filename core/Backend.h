#pragma once

#include <memory>
#include <vector>

namespace ir {

class Expr;

} // namespace ir

class HeightMapObserver;

class Backend
{
public:
  static auto MakeCpuBackend() -> std::unique_ptr<Backend>;

  virtual ~Backend() = default;

  virtual void AddHeightMapObserver(std::unique_ptr<HeightMapObserver>) = 0;

  virtual void ComputeHeightMap() = 0;

  virtual void ComputeSurface() = 0;

  virtual void Resize(size_t w, size_t h) = 0;

  /// @note @p buf must be large enough to fit
  /// the entire height map.
  virtual void ReadHeightMap(float* buf) const = 0;

  /// @brief Updates the internal expression that represents height.
  ///
  /// @param heightExpr This can either be a valid height expression or
  /// it can be null, in which case a zero constant replaces the current
  /// expression.
  virtual bool UpdateHeightExpr(const ir::Expr* heightExpr) = 0;

  virtual bool UpdateColorExpr(const ir::Expr* colorExpr) = 0;
};
