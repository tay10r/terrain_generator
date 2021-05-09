#pragma once

#include "core/Backend.h"

class CpuBackend : public Backend
{
public:
  static auto Make() -> CpuBackend*;

  virtual ~CpuBackend() = default;
};
