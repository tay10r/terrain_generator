#include <gtest/gtest.h>

#include "Backend.h"
#include "ExprTests.h"

TEST(CpuBackend, ExprTests)
{
  auto exprTests = ExprTests::All();

  for (const auto& exprTest : exprTests) {

    SCOPED_TRACE(exprTest->GetName());

    auto cpuEngine = Backend::MakeCpuBackend();

    exprTest->Run(*cpuEngine);
  }
}
