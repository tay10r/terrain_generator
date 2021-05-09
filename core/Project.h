#pragma once

namespace ir {

class Expr;

} // namespace ir

class Project
{
public:
  virtual ~Project() = default;

  virtual auto GetHeightExpr() const -> const ir::Expr* = 0;

  virtual auto GetColorExpr() const -> const ir::Expr* = 0;

  virtual auto GetTerrainWidth() const noexcept -> size_t = 0;

  virtual auto GetTerrainHeight() const noexcept -> size_t = 0;
};
