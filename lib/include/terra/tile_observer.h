#pragma once

namespace terra {

class Tile;

class TileObserver
{
public:
  virtual ~TileObserver() = default;

  virtual void Observe(const Tile&) = 0;
};

} // namespace terra
