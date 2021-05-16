#pragma once

namespace terra {

enum class Type
{
  Float,
  Int,
  Vec2,
  Vec3,
  Vec4,
  Mat2,
  Mat3,
  Mat4
};

template<typename ValueType>
struct TypeMap final
{};

template<>
struct TypeMap<int> final
{
  static constexpr Type GetType() noexcept { return Type::Int; }
};

template<>
struct TypeMap<float> final
{
  static constexpr Type GetType() noexcept { return Type::Float; }
};

} // namespace terra
