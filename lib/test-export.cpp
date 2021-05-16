#include <terra/interpreter.h>
#include <terra/png_writer.h>

#include <terra/exprs/literals.h>
#include <terra/exprs/var_ref.h>
#include <terra/exprs/vector_combiner.h>

#include <iostream>

namespace {

using SharedExprPtr = std::shared_ptr<terra::Expr>;

auto
MakeColorExpr() -> SharedExprPtr
{
  auto r = SharedExprPtr(new terra::VarRefExpr(terra::VarRefExpr::ID::CenterU));
  auto g = SharedExprPtr(new terra::VarRefExpr(terra::VarRefExpr::ID::CenterV));
  auto b = SharedExprPtr(new terra::LiteralExpr<float>(1.0));

  std::array<SharedExprPtr, 3> elements;
  elements[0] = std::move(r);
  elements[1] = std::move(g);
  elements[2] = std::move(b);

  return SharedExprPtr(new terra::VectorCombiner<3>(std::move(elements)));
}

auto
MakeHeightExpr() -> SharedExprPtr
{
  return SharedExprPtr(new terra::VarRefExpr(terra::VarRefExpr::ID::CenterU));
}

} // namespace

int
main()
{
  auto heightExpr = MakeHeightExpr();

  auto colorExpr = MakeColorExpr();

  size_t w = 1024;
  size_t h = 1024;

  auto pngWriter = terra::PngWriter::Make(w, h, "height.png", "color.png");

  auto interpreter = terra::LineInterpreter::Make(w, h, *pngWriter);

  interpreter->SetHeightExpr(*heightExpr);

  interpreter->SetColorExpr(*colorExpr);

  interpreter->Execute();

  return 0;
}
