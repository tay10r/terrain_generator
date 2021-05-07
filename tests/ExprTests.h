#pragma once

#include <memory>
#include <vector>

class Backend;

class ExprTest
{
public:
  virtual ~ExprTest() = default;

  virtual const char* GetName() const noexcept = 0;

  virtual void Run(Backend& backend) = 0;
};

class ExprTests final
{
public:
  static auto All() -> std::vector<std::unique_ptr<ExprTest>>;
};
