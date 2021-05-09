#pragma once

#include "gui/ForwardDecl.h"

class ExprObserver
{
public:
  virtual ~ExprObserver() = default;

  virtual void Observe(const ir::Expr&) = 0;
};
