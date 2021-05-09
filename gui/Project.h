#pragma once

class Project final
{
public:
  virtual ~Project() = default;

  virtual size_t GetTerrainWidth() const noexcept = 0;

  virtual size_t GetTerrainHeight() const noexcept = 0;
};
