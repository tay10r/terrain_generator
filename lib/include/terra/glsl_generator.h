#pragma once

#include <iosfwd>

namespace terra {

class ExprVisitor;

auto
MakeGlslGenerator(std::ostream&) -> std::unique_ptr<ExprVisitor>;

} // namespace terra
