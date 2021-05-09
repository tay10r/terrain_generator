#include "ExprTests.h"

#include "core/Backend.h"
#include "core/IR.h"

#include <gtest/gtest.h>

namespace {

constexpr size_t w = 4;

constexpr size_t h = 4;

using Row = std::array<float, w>;

using HeightMap = std::vector<float>;

class ExprTestBase : public ExprTest
{
public:
  virtual ir::Expr* BuildExpr() const = 0;

  virtual void CheckHeightMap(const HeightMap&) const = 0;

  void Run(Backend& backend) override
  {
    std::unique_ptr<ir::Expr> expr(BuildExpr());

    backend.Resize(w, h);

    backend.UpdateHeightExpr(expr.get());

    backend.ComputeHeightMap();

    HeightMap heightMap(w * h);

    backend.ReadHeightMap(&heightMap[0]);

    CheckHeightMap(heightMap);
  }

  void ExpectRow(const HeightMap& hMap, size_t rowIndex, const Row& row) const
  {
    const float bias = 1.0f / 0xffff;

    for (size_t i = 0; i < w; i++) {
      EXPECT_NEAR(hMap[(rowIndex * w) + i], row[i], bias);
    }
  }
};

class UCoordTest final : public ExprTestBase
{
public:
  const char* GetName() const noexcept override { return "UCoordTest"; }

  void CheckHeightMap(const HeightMap& heightMap) const override
  {
    ExpectRow(heightMap, 0, { 0.125, 0.375, 0.625, 0.875 });
    ExpectRow(heightMap, 1, { 0.125, 0.375, 0.625, 0.875 });
    ExpectRow(heightMap, 2, { 0.125, 0.375, 0.625, 0.875 });
    ExpectRow(heightMap, 3, { 0.125, 0.375, 0.625, 0.875 });
  }

  ir::Expr* BuildExpr() const override
  {
    return new ir::VarRefExpr(ir::VarRefExpr::ID::CenterUCoord);
  }
};

class VCoordTest final : public ExprTestBase
{
public:
  const char* GetName() const noexcept override { return "UCoordTest"; }

  void CheckHeightMap(const HeightMap& heightMap) const override
  {
    ExpectRow(heightMap, 0, { 0.125, 0.125, 0.125, 0.125 });
    ExpectRow(heightMap, 1, { 0.375, 0.375, 0.375, 0.375 });
    ExpectRow(heightMap, 2, { 0.625, 0.625, 0.625, 0.625 });
    ExpectRow(heightMap, 3, { 0.875, 0.875, 0.875, 0.875 });
  }

  ir::Expr* BuildExpr() const override
  {
    return new ir::VarRefExpr(ir::VarRefExpr::ID::CenterVCoord);
  }
};

} // namespace

auto
ExprTests::All() -> std::vector<std::unique_ptr<ExprTest>>
{
  std::vector<std::unique_ptr<ExprTest>> tests;

  tests.emplace_back(new UCoordTest());
  tests.emplace_back(new VCoordTest());

  return tests;
}
