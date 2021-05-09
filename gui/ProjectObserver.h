#pragma once

namespace ir {

class Expr;

} // namespace ir

class ProjectObserver
{
public:
  virtual ~ProjectObserver() = default;

  /// @brief Whenever the height expression changes,
  /// this function gets called.
  ///
  /// @note @p heightExpr may be null if the expression is either
  /// invalid or incomplete.
  virtual void ObserveHeightChange(const ir::Expr* heightExpr) = 0;

  virtual void ObserveSurfaceChange(const ir::Expr* colorExpr) = 0;
};
