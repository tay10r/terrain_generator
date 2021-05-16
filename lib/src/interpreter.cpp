#include <terra/interpreter.h>

#include <terra/expr_visitor.h>
#include <terra/line_observer.h>
#include <terra/tile.h>
#include <terra/tile_observer.h>

#include <terra/exprs/literals.h>
#include <terra/exprs/var_ref.h>
#include <terra/exprs/vector_combiner.h>

#include <algorithm>
#include <type_traits>
#include <vector>

#include <Eigen/Dense>

namespace terra {

namespace {

template<typename Scalar>
Scalar
Clamp(Scalar x, Scalar min, Scalar max) noexcept
{
  return std::min(std::max(x, min), max);
}

struct BuiltinVars final
{
  float uCenter;
  float vCenter;
};

namespace impl {

template<typename Scalar, size_t Size>
using Vector = Eigen::Matrix<Scalar, Size, 1>;

template<typename Type>
class Expr
{
public:
  virtual ~Expr() = default;

  virtual Type Eval(const BuiltinVars&) const noexcept = 0;
};

template<typename Scalar, size_t Size>
using VectorExpr = Expr<Vector<Scalar, Size>>;

class UCenterExpr final : public Expr<float>
{
public:
  float Eval(const BuiltinVars& builtinVars) const noexcept override
  {
    return builtinVars.uCenter;
  }
};

class VCenterExpr final : public Expr<float>
{
public:
  float Eval(const BuiltinVars& builtinVars) const noexcept override
  {
    return builtinVars.vCenter;
  }
};

template<typename Scalar, size_t Size>
class VectorCombinerExpr final : public VectorExpr<Scalar, Size>
{
public:
  VectorCombinerExpr(std::array<std::unique_ptr<Expr<Scalar>>, Size> elements)
    : mElements(std::move(elements))
  {}

  auto Eval(const BuiltinVars& builtinVars) const noexcept
    -> Vector<Scalar, Size> override
  {
    Vector<Scalar, Size> result;

    for (size_t i = 0; i < Size; i++)
      result(i) = mElements[i]->Eval(builtinVars);

    return result;
  }

private:
  std::array<std::unique_ptr<Expr<Scalar>>, Size> mElements;
};

template<typename Scalar>
class LiteralExpr final : public Expr<Scalar>
{
public:
  LiteralExpr(Scalar value) noexcept
    : mValue(value)
  {}

  Scalar Eval(const BuiltinVars&) const noexcept override { return mValue; }

private:
  Scalar mValue;
};

} // namespace impl

template<typename Type>
class ExprBuilder final : public ExprVisitor
{
public:
  auto TakeExpr() -> std::unique_ptr<impl::Expr<Type>>
  {
    return std::move(mExpr);
  }

  void Visit(const VarRefExpr& varRef) override
  {
    switch (varRef.GetID()) {
      case VarRefExpr::ID::CenterU:
        if constexpr (std::is_same<Type, float>::value)
          mExpr.reset(new impl::UCenterExpr());
        break;
      case VarRefExpr::ID::CenterV:
        if constexpr (std::is_same<Type, float>::value)
          mExpr.reset(new impl::VCenterExpr());
        break;
    }
  }

  void Visit(const IntLiteralExpr& intLiteral) override
  {
    if constexpr (std::is_same<Type, int>::value)
      mExpr.reset(new impl::LiteralExpr<int>(intLiteral.GetValue()));
  }

  void Visit(const FloatLiteralExpr& floatLiteral) override
  {
    if constexpr (std::is_same<Type, float>::value)
      mExpr.reset(new impl::LiteralExpr<float>(floatLiteral.GetValue()));
  }

  void Visit(const FloatToIntExpr&) override {}

  void Visit(const IntToFloatExpr&) override {}

  void Visit(const UnaryExpr&) override {}

  void Visit(const BinaryExpr&) override {}

  void Visit(const VectorCombiner<2>& vecCombiner) override
  {
    if constexpr (std::is_same<Type, impl::Vector<float, 2>>::value)
      HandleVectorCombiner(vecCombiner);
  }

  void Visit(const VectorCombiner<3>& vecCombiner) override
  {
    if constexpr (std::is_same<Type, impl::Vector<float, 3>>::value)
      HandleVectorCombiner(vecCombiner);
  }

  void Visit(const VectorCombiner<4>& vecCombiner) override
  {
    if constexpr (std::is_same<Type, impl::Vector<float, 4>>::value)
      HandleVectorCombiner(vecCombiner);
  }

private:
  template<size_t Size>
  void HandleVectorCombiner(const VectorCombiner<Size>& vecCombiner)
  {
    std::array<std::unique_ptr<impl::Expr<float>>, Size> elements;

    for (size_t i = 0; i < Size; i++) {

      ExprBuilder<float> elementBuilder;

      vecCombiner.GetElement(i).Accept(elementBuilder);

      elements[i] = elementBuilder.TakeExpr();
    }

    mExpr.reset(new impl::VectorCombinerExpr<float, Size>(std::move(elements)));
  }

private:
  std::unique_ptr<impl::Expr<Type>> mExpr;
};

struct FrameStatus final
{
  FrameStatus(size_t tilesPerRow, size_t tilesPerCol, size_t w, size_t h)
    : tileCount(tilesPerRow * tilesPerCol)
    , tileIndex(0)
    , resX(w)
    , resY(h)
  {}

  size_t tileCount = 0;

  size_t tileIndex = 0;

  size_t resX = 0;

  size_t resY = 0;

  size_t GetNextXOffset() const noexcept
  {
    return (tileIndex * TileSize()) % resX;
  }

  size_t GetNextYOffset() const noexcept
  {
    return ((tileIndex * TileSize()) / resX) * TileSize();
  }

  size_t GetNextWidth() const noexcept
  {
    auto offset = GetNextXOffset();

    auto end = offset + TileSize();

    return std::min(end, resX) - offset;
  }

  size_t GetNextHeight() const noexcept
  {
    auto offset = GetNextYOffset();

    auto end = offset + TileSize();

    return std::min(end, resY) - offset;
  }
};

class RenderTask final
{
public:
  RenderTask(Tile& tile, const impl::Expr<float>& heightExpr)
    : mTile(tile)
    , mHeightExpr(heightExpr)
  {}

  void operator()() noexcept
  {
    auto& buffer = mTile.GetBuffer();

    for (size_t i = 0; i < (TileSize() * TileSize()); i++) {

      size_t x = i % TileSize();
      size_t y = i / TileSize();

      BuiltinVars builtinVars;
      builtinVars.uCenter = (x + 0.5f) / TileSize();
      builtinVars.vCenter = (y + 0.5f) / TileSize();

      auto h = mHeightExpr.Eval(builtinVars);

      buffer[(i * 4) + 0] = h;
      buffer[(i * 4) + 1] = 0;
      buffer[(i * 4) + 2] = 0;
      buffer[(i * 4) + 3] = 0;
    }
  }

private:
  Tile& mTile;

  const impl::Expr<float>& mHeightExpr;
};

class TileInterpreterImpl final : public TileInterpreter
{
public:
  void AddTileObserver(std::shared_ptr<TileObserver> observer) override
  {
    mTileObservers.emplace_back(std::move(observer));
  }

  bool FrameIsDone() const noexcept override { return TilesRemaining() == 0; }

  size_t TilesRemaining() const noexcept override
  {
    if (!mFrameStatus)
      return 0;

    return mFrameStatus->tileCount - mFrameStatus->tileIndex;
  }

  bool BeginFrame() override
  {
    if (mFrameStatus)
      return false;

    size_t tilesPerRow = (mResX + (TileSize() - 1)) / TileSize();

    size_t tilesPerCol = (mResY + (TileSize() - 1)) / TileSize();

    mFrameStatus.reset(new FrameStatus(tilesPerRow, tilesPerCol, mResX, mResY));

    return true;
  }

  bool PollTiles(size_t) override
  {
    if (!mFrameStatus || !mHeightExpr)
      return false;

    auto x = mFrameStatus->GetNextXOffset();
    auto y = mFrameStatus->GetNextYOffset();
    auto w = mFrameStatus->GetNextWidth();
    auto h = mFrameStatus->GetNextHeight();

    Tile tile(x, y, w, h);

    RenderTask renderTask(tile, *mHeightExpr);

    renderTask();

    NotifyTileObservers(tile);

    mFrameStatus->tileIndex++;

    return true;
  }

  bool EndFrame() override
  {
    if (!mFrameStatus)
      return false;

    mFrameStatus.reset();

    return true;
  }

  bool SetHeightExpr(const terra::Expr& heightExpr) override
  {
    ExprBuilder<float> exprBuilder;

    heightExpr.Accept(exprBuilder);

    mHeightExpr = exprBuilder.TakeExpr();

    return !!mHeightExpr;
  }

  void SetResolution(size_t w, size_t h) override
  {
    mResX = w;
    mResY = h;
  }

private:
  void NotifyTileObservers(const Tile& tile)
  {
    for (auto& tileObserver : mTileObservers)
      tileObserver->Observe(tile);
  }

private:
  size_t mResX = 0;

  size_t mResY = 0;

  std::vector<std::shared_ptr<TileObserver>> mTileObservers;

  std::unique_ptr<FrameStatus> mFrameStatus;

  std::unique_ptr<impl::Expr<float>> mHeightExpr;
};

} // namespace

auto
TileInterpreter::Make() -> std::unique_ptr<TileInterpreter>
{
  return std::unique_ptr<TileInterpreter>(new TileInterpreterImpl());
}

namespace {

class LineInterpreterImpl final : public LineInterpreter
{
public:
  LineInterpreterImpl(size_t w, size_t h, LineObserver& lineObserver)
    : mWidth(w)
    , mHeight(h)
    , mLineObserver(lineObserver)
  {}

  bool SetHeightExpr(const Expr& heightExpr) override
  {
    ExprBuilder<float> exprBuilder;

    heightExpr.Accept(exprBuilder);

    mHeightExpr = exprBuilder.TakeExpr();

    return true;
  }

  bool SetColorExpr(const Expr& expr) override
  {
    ExprBuilder<impl::Vector<float, 3>> exprBuilder;

    expr.Accept(exprBuilder);

    mColorExpr = exprBuilder.TakeExpr();

    return true;
  }

  bool Execute() override
  {
    if (!mHeightExpr || !mColorExpr)
      return false;

    std::vector<float> heightAndRgbBuffer(mWidth * 4);

    for (size_t y = 0; y < mHeight; y++) {

      for (size_t x = 0; x < mWidth; x++) {

        BuiltinVars builtinVars;
        builtinVars.uCenter = (x + 0.5f) / mWidth;
        builtinVars.vCenter = (y + 0.5f) / mHeight;

        auto h = mHeightExpr->Eval(builtinVars);

        auto c = mColorExpr->Eval(builtinVars);

        heightAndRgbBuffer[(x * 4) + 0] = h;
        heightAndRgbBuffer[(x * 4) + 1] = Clamp(c(0) * 255.0f, 0.0f, 255.0f);
        heightAndRgbBuffer[(x * 4) + 2] = Clamp(c(1) * 255.0f, 0.0f, 255.0f);
        heightAndRgbBuffer[(x * 4) + 3] = Clamp(c(2) * 255.0f, 0.0f, 255.0f);
      }

      mLineObserver.Observe(heightAndRgbBuffer.data());
    }

    return true;
  }

private:
  size_t mWidth;

  size_t mHeight;

  LineObserver& mLineObserver;

  std::unique_ptr<impl::Expr<float>> mHeightExpr;

  std::unique_ptr<impl::Expr<impl::Vector<float, 3>>> mColorExpr;
};

} // namespace

auto
LineInterpreter::Make(size_t w, size_t h, LineObserver& lineObserver)
  -> std::unique_ptr<LineInterpreter>
{
  using Ret = std::unique_ptr<LineInterpreter>;

  return Ret(new LineInterpreterImpl(w, h, lineObserver));
}

} // namespace terra
