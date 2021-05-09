#include "core/Backend.h"

#include "CpuBackend.h"

auto
Backend::MakeCpuBackend() -> std::unique_ptr<Backend>
{
  return std::unique_ptr<Backend>(CpuBackend::Make());
}
