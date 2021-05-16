#pragma once

#include <memory>

namespace terra {

class Expr;
class TileObserver;
class LineObserver;

/// Used to render terrain in tiles, using portable C++.
/// Useful for displaying terrain in a 3D view.
class TileInterpreter
{
public:
  static auto Make() -> std::unique_ptr<TileInterpreter>;

  virtual ~TileInterpreter() = default;

  virtual void AddTileObserver(std::shared_ptr<TileObserver>) = 0;

  /// After calling @ref Interpreter::BeginFrame, this function will indicate
  /// whether or not the last tile has been rendered. If a frame is not
  /// currently being rendered, then true is returned.
  virtual bool FrameIsDone() const noexcept = 0;

  /// Indicates the number of tiles that still need to be rendered within a
  /// frame.
  ///
  /// @return The number of remaining tiles. If no frame is currently being
  /// rendered, then this function returns zero.
  virtual size_t TilesRemaining() const noexcept = 0;

  /// Begins rendering a frame.
  ///
  /// @return True on success, false if a frame is currently being rendered.
  virtual bool BeginFrame() = 0;

  /// Checks for completed tiles.
  ///
  /// @param timeout The number of milliseconds to wait before returning.
  ///
  /// @return True on success, false if no frame is currently being rendered.
  virtual bool PollTiles(size_t timeout) = 0;

  /// Completes the frame rendering process.
  ///
  /// @return True on success, false if no frame is currently being rendered.
  virtual bool EndFrame() = 0;

  /// Assigns the expression that represents the elevation of a terrain.
  ///
  /// @param expr The expression representing height per coordinate.
  ///             Should return a single floating point value.
  ///
  /// @return False on failure, true on success. This function may fail if the
  /// expression has a type error or if the interpreter is currently rendering
  /// a frame.
  virtual bool SetHeightExpr(const Expr&) = 0;

  virtual void SetResolution(size_t w, size_t h) = 0;
};

/// Used to render terrain by scanline, using portable C++.
/// Useful for saving terrain to an image.
class LineInterpreter
{
public:
  static auto Make(size_t w, size_t h, LineObserver&)
    -> std::unique_ptr<LineInterpreter>;

  virtual ~LineInterpreter() = default;

  virtual bool SetHeightExpr(const Expr& heightExpr) = 0;

  virtual bool SetColorExpr(const Expr& colorExpr) = 0;

  virtual bool Execute() = 0;
};

} // namespace terra
