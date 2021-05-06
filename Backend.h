#pragma once

#include <memory>

namespace ir {

class Expr;

} // namespace ir

class Backend
{
public:
  static auto MakeCpuBackend() -> std::unique_ptr<Backend>;

  virtual ~Backend() = default;

  virtual void ComputeHeightMap() = 0;

  virtual void Display() = 0;

  virtual void Resize(size_t w, size_t h) = 0;

  virtual bool UpdateHeightMapExpr(const ir::Expr&) = 0;
};
