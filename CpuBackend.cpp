#include "CpuBackend.h"

#include "IR.h"
#include "OpenGL.h"

#include <glm/gtc/matrix_transform.hpp>

#include <fstream>
#include <iostream>
#include <vector>

#include <stdint.h>

const char* gVertShaderSource = R"(
#version 110

attribute vec3 position;

varying vec2 tex_coord;

uniform mat4 mvp;

void
main()
{
    gl_Position = mvp * vec4(position, 1.0);

    tex_coord = (position.xy + 1.0) * 0.5;
}
)";

const char* gFragShaderSource = R"(
#version 110

varying vec2 tex_coord;

void
main()
{
    gl_FragColor = vec4(tex_coord, 1.0, 1.0);
}
)";

namespace {

struct BuiltinVars final
{
  float u;
  float v;
};

class FloatExpr
{
public:
  virtual ~FloatExpr() = default;

  virtual float Eval(const BuiltinVars&) const noexcept = 0;
};

class FloatLiteral final : public FloatExpr
{
public:
  FloatLiteral(float value)
    : mValue(value)
  {}

  float Eval(const BuiltinVars&) const noexcept override { return mValue; }

private:
  float mValue;
};

class UCoordExpr final : public FloatExpr
{
public:
  float Eval(const BuiltinVars& builtinVars) const noexcept override
  {
    return builtinVars.u;
  }
};

class VCoordExpr final : public FloatExpr
{
public:
  float Eval(const BuiltinVars& builtinVars) const noexcept override
  {
    return builtinVars.v;
  }
};

class FloatExprBuilder : public ir::ExprVisitor
{
public:
  auto TakeResult() -> std::unique_ptr<FloatExpr>
  {
    return std::move(mFloatExpr);
  }

  void Visit(const ir::VarRefExpr& varRefExpr) override
  {
    switch (varRefExpr.GetID()) {
      case ir::VarRefExpr::ID::CenterUCoord:
        mFloatExpr.reset(new UCoordExpr());
        break;
      case ir::VarRefExpr::ID::CenterVCoord:
        mFloatExpr.reset(new VCoordExpr());
        break;
    }
  }

  void Visit(const ir::FloatLiteralExpr& floatLiteralExpr) override
  {
    mFloatExpr.reset(new FloatLiteral(floatLiteralExpr.GetValue()));
  }

  void Visit(const ir::IntLiteralExpr&) override {}

private:
  std::unique_ptr<FloatExpr> mFloatExpr;
};

class GlContext final
{
public:
  GlContext()
    : mProgram(CreateShaderProgram())
  {
    glGenBuffers(1, &mVertexBufferID);

    glGenBuffers(1, &mIndexBufferID);

    mMvpLocation = glGetUniformLocation(mProgram.ID(), "mvp");
  }

  void UpdateBufferData(const std::vector<float>& height, size_t w, size_t h)
  {
    UpdateVertexBuffer(height, w, h);

    UpdateIndexBuffer(w, h);
  }

  void Render(size_t w, size_t h)
  {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glBindBuffer(GL_ARRAY_BUFFER, mVertexBufferID);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexBufferID);

    glUseProgram(mProgram.ID());

    if (mMvpLocation != -1) {

      float aspect = float(w) / h;

      float t_near = 0.001;

      float t_far = 100;

      auto proj = glm::perspective(glm::radians(45.0f), aspect, t_near, t_far);

      auto eye = glm::vec3(3, 3, 3);
      auto center = glm::vec3(0, 0, 0);
      auto up = glm::vec3(0, 1, 0);
      auto view = glm::lookAt(eye, center, up);

      auto model = glm::mat4(1.0f);

      auto mvp = proj * view * model;

      glUniformMatrix4fv(mMvpLocation, 1, GL_FALSE, &mvp[0][0]);
    }

    glDrawElements(GL_TRIANGLES, (w - 1) * (h - 1) * 6, GL_UNSIGNED_INT, 0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  }

private:
  void UpdateVertexBuffer(const std::vector<float>& height, size_t w, size_t h)
  {
    mVertexBuffer.resize(w * h * 3);

    for (size_t i = 0; i < (w * h); i++) {

      size_t x = i % w;
      size_t y = i / w;

      mVertexBuffer[(i * 3) + 0] = (((x + 0.5f) / w) * 2.0f) - 1.0f;
      mVertexBuffer[(i * 3) + 1] = height[i];
      mVertexBuffer[(i * 3) + 2] = (((y + 0.5f) / h) * 2.0f) - 1.0f;
    }

    glBindBuffer(GL_ARRAY_BUFFER, mVertexBufferID);

    auto posAttribLoc = glGetAttribLocation(mProgram.ID(), "position");

    glEnableVertexAttribArray(posAttribLoc);

    glVertexAttribPointer(
      posAttribLoc, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);

    glBufferData(GL_ARRAY_BUFFER,
                 mVertexBuffer.size() * sizeof(mVertexBuffer[0]),
                 mVertexBuffer.data(),
                 GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
  }

  void UpdateIndexBuffer(size_t w, size_t h)
  {
    InitializeIndexBuffer(w, h);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexBufferID);

    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 mIndexBuffer.size() * sizeof(mIndexBuffer[0]),
                 mIndexBuffer.data(),
                 GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  }

  void InitializeIndexBuffer(size_t w, size_t h)
  {
    size_t bufW = w - 1;
    size_t bufH = h - 1;

    mIndexBuffer.resize(bufW * bufH * 6);

    auto toVertIndex = [w](size_t x, size_t y) { return (y * w) + x; };

    auto* ptr = mIndexBuffer.data();

    for (size_t i = 0; i < (bufW * bufH); i++) {

      size_t x = i % bufW;
      size_t y = i / bufW;

      ptr[0] = toVertIndex(x, y);
      ptr[1] = toVertIndex(x, y + 1);
      ptr[2] = toVertIndex(x + 1, y);

      ptr[3] = toVertIndex(x, y + 1);
      ptr[4] = toVertIndex(x + 1, y + 1);
      ptr[5] = toVertIndex(x + 1, y);

      ptr += 6;
    }
  }

  static auto CreateShaderProgram() -> GlProgram
  {
    auto vertShader = GlShader::Create(GL_VERTEX_SHADER, gVertShaderSource);

    std::pair<bool, std::string> compResult = vertShader.GetCompileResult();

    if (!compResult.first) {
      std::cerr << "vertex shader: " << compResult.second << std::endl;
      return GlProgram::MakeInvalid();
    }

    auto fragShader = GlShader::Create(GL_FRAGMENT_SHADER, gFragShaderSource);

    compResult = fragShader.GetCompileResult();

    if (!compResult.first) {
      std::cerr << "fragment shader: " << compResult.second << std::endl;
      return GlProgram::MakeInvalid();
    }

    auto prg = GlProgram::Create({ vertShader.ID(), fragShader.ID() });

    auto linkResult = prg.GetLinkResult();

    if (linkResult.first)
      return prg;

    std::cerr << linkResult.second << std::endl;

    return prg;
  }

private:
  std::vector<float> mVertexBuffer;

  std::vector<uint32_t> mIndexBuffer;

  GLuint mVertexBufferID = 0;

  GLuint mIndexBufferID = 0;

  GlProgram mProgram = GlProgram::MakeInvalid();

  GLint mMvpLocation = 0;
};

class CpuBackendImpl final : public CpuBackend
{
public:
  CpuBackendImpl() {}

  void ComputeHeightMap() override
  {
    if (!mHeightMapExpr)
      mHeightMapExpr.reset(new FloatLiteral(0.0));

    BuiltinVars builtinVars;

    for (size_t i = 0; i < (mWidth * mHeight); i++) {

      size_t x = i % mWidth;
      size_t y = i / mWidth;

      builtinVars.u = (x + 0.5f) / mWidth;
      builtinVars.v = (y + 0.5f) / mHeight;

      mHeightMap[i] = mHeightMapExpr->Eval(builtinVars);
    }
  }

  void Display() override
  {
    if (!mGlContext)
      mGlContext.reset(new GlContext());

    mGlContext->UpdateBufferData(mHeightMap, mWidth, mHeight);

    mGlContext->Render(mWidth, mHeight);
  }

  void Resize(size_t w, size_t h) override
  {
    mHeightMap.resize(w * h);

    mWidth = w;

    mHeight = h;
  }

  bool UpdateHeightMapExpr(const ir::Expr& expr) override
  {
    FloatExprBuilder floatExprBuilder;

    expr.Accept(floatExprBuilder);

    auto result = floatExprBuilder.TakeResult();

    if (!result)
      return false;

    mHeightMapExpr = std::move(result);

    return true;
  }

  void ReadHeightMap(float* buf) const override
  {
    for (size_t i = 0; i < mHeightMap.size(); i++)
      buf[i] = mHeightMap[i];
  }

private:
  std::unique_ptr<GlContext> mGlContext;

  std::unique_ptr<FloatExpr> mHeightMapExpr;

  std::vector<float> mHeightMap;

  size_t mWidth = 0;

  size_t mHeight = 0;
};

} // namespace

auto
CpuBackend::Make() -> CpuBackend*
{
  return new CpuBackendImpl();
}
