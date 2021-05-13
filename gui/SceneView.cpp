#include "gui/SceneView.h"

#include "core/Camera.h"
#include "core/HeightMapObserver.h"

#include "OpenGL.h"

#include <glm/gtx/transform.hpp>

#include <QOpenGLWidget>

#include <iostream>

namespace {

const char*
GlErrorToString(GLenum error)
{
  switch (error) {
    case GL_NO_ERROR:
      return "<no-error>";
    case GL_INVALID_ENUM:
      return "invalid enum";
    case GL_INVALID_VALUE:
      return "invalid value";
    case GL_INVALID_OPERATION:
      return "invalid operation";
  }

  return "unknown error";
}

void
DumpGlError(unsigned int line, const char* expr, GLenum error)
{
  std::cerr << "OpenGL Error:" << std::endl;
  std::cerr << "   file : " << __FILE__ << std::endl;
  std::cerr << "   line : " << line << std::endl;
  std::cerr << "   expr : " << expr << std::endl;
  std::cerr << "  error : " << GlErrorToString(error) << std::endl;
}

} // namespace

#define CHECK_GL_CALL(callExpr)                                                \
  do {                                                                         \
    (callExpr);                                                                \
    auto error = glGetError();                                                 \
    if (error != GL_NO_ERROR)                                                  \
      DumpGlError(__LINE__, #callExpr, error);                                 \
  } while (0)

namespace {

const char* gVertShaderSource = R"(
#version 110

attribute float gLocationIndex;

attribute float gHeight;

uniform mat4 gMVP;

uniform float gTerrainWidth;

uniform float gTerrainHeight;

varying vec2 gTexCoord;

void
main()
{
  float x = mod(gLocationIndex, gTerrainWidth);

  float y = floor(gLocationIndex / gTerrainWidth);

  float uCenter = (x + 0.5) / gTerrainWidth;

  float vCenter = (y + 0.5) / gTerrainHeight;

  gTexCoord = vec2(uCenter, vCenter);

  float ndcX = uCenter * 2.0 - 1.0;

  float ndcY = 1.0f - vCenter * 2.0;

  gl_Position = gMVP * vec4(ndcX, gHeight, ndcY, 1.0);
}
)";

const char* gFragShaderSource = R"(
#version 110

varying vec2 gTexCoord;

void
main()
{
  gl_FragColor = vec4(gTexCoord, 1.0, 1.0);
}
)";

class Shader final
{
public:
  Shader()
    : mProgram(CreateShaderProgram())
  {
    glGenBuffers(1, &mVertexBufferID);

    glGenBuffers(1, &mElementBufferID);

    CHECK_GL_CALL(glUseProgram(mProgram.ID()));

    CHECK_GL_CALL(glActiveTexture(GL_TEXTURE0));

    mMvpLocation = glGetUniformLocation(mProgram.ID(), "gMVP");

    mWidthLocation = glGetUniformLocation(mProgram.ID(), "gTerrainWidth");

    mHeightLocation = glGetUniformLocation(mProgram.ID(), "gTerrainHeight");

    UpdateBufferSize(2, 2);

    UpdateCameraData(1.0f);

    const float initialHeight[4]{ 0, 0, 0, 0 };

    UpdateHeightMap(initialHeight, 2, 2);
  }

  void UpdateHeightMap(const float* data, size_t w, size_t h)
  {
    glBindBuffer(GL_ARRAY_BUFFER, mVertexBufferID);

    auto attrib = glGetAttribLocation(mProgram.ID(), "gHeight");

    glEnableVertexAttribArray(attrib);

    using ConstVoidPtr = const void*;

    auto offset = w * h * sizeof(float);

    auto offsetPtr = ConstVoidPtr(offset);

    auto stride = sizeof(float);

    glVertexAttribPointer(attrib, 1, GL_FLOAT, GL_FALSE, stride, offsetPtr);

    glBufferSubData(GL_ARRAY_BUFFER, offset, w * h * sizeof(float), data);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
  }

  void UpdateCameraData(float aspect)
  {
    if (mMvpLocation == -1)
      return;

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

  void UpdateBufferSize(size_t w, size_t h)
  {
    UpdateVertexBuffer(w, h);

    UpdateElementBuffer(w, h);

    glUniform1f(mWidthLocation, float(w));

    glUniform1f(mHeightLocation, float(h));

    mIndexCount = (w - 1) * (h - 1) * 6;
  }

  void Render()
  {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    CHECK_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, mVertexBufferID));

    CHECK_GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mElementBufferID));

    CHECK_GL_CALL(
      glDrawElements(GL_TRIANGLES, mIndexCount, GL_UNSIGNED_INT, 0));

    CHECK_GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));

    CHECK_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, 0));
  }

private:
  void UpdateVertexBuffer(size_t w, size_t h)
  {
    std::vector<float> indexBuffer(w * h);

    for (size_t i = 0; i < (w * h); i++)
      indexBuffer[i] = i;

    glBindBuffer(GL_ARRAY_BUFFER, mVertexBufferID);

    // The size here is one location value and one height value per vertex
    auto bufferSize = w * h * 2 * sizeof(float);

    glBufferData(GL_ARRAY_BUFFER, bufferSize, nullptr, GL_DYNAMIC_DRAW);

    auto posAttrib = glGetAttribLocation(mProgram.ID(), "gLocationIndex");

    glEnableVertexAttribArray(posAttrib);

    glVertexAttribPointer(posAttrib, 1, GL_FLOAT, GL_FALSE, sizeof(float), 0);

    glBufferSubData(GL_ARRAY_BUFFER,
                    0,
                    indexBuffer.size() * sizeof(indexBuffer[0]),
                    indexBuffer.data());

    glBindBuffer(GL_ARRAY_BUFFER, 0);
  }

  void UpdateElementBuffer(size_t w, size_t h)
  {
    auto buf = InitializeElementBuffer(w, h);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mElementBufferID);

    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 buf.size() * sizeof(buf[0]),
                 buf.data(),
                 GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  }

  static auto InitializeElementBuffer(size_t w, size_t h)
    -> std::vector<uint32_t>
  {
    size_t bufW = w - 1;
    size_t bufH = h - 1;

    std::vector<uint32_t> buf(bufW * bufH * 6);

    auto toVertIndex = [w](size_t x, size_t y) { return (y * w) + x; };

    auto* ptr = buf.data();

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

    return buf;
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
  GLuint mVertexBufferID = 0;

  GLuint mElementBufferID = 0;

  GlProgram mProgram = GlProgram::MakeInvalid();

  GLint mMvpLocation = -1;

  GLint mWidthLocation = -1;

  GLint mHeightLocation = -1;

  /// The number of vertex indices.
  /// Only changes when the terrain size is changed.
  GLsizei mIndexCount = 0;
};

struct HeightMapUpdate final
{
  const float* data = nullptr;
  size_t w = 0;
  size_t h = 0;
  bool seen = true;
};

class HeightMapUpdater final : public HeightMapObserver
{
public:
  HeightMapUpdater(QOpenGLWidget* glWidget,
                   std::shared_ptr<HeightMapUpdate> update)
    : mGlWidget(glWidget)
    , mHeightMapUpdate(update)
  {}

  void Observe(const float* data, size_t w, size_t h) override
  {
    mHeightMapUpdate->data = data;
    mHeightMapUpdate->w = w;
    mHeightMapUpdate->h = h;
    mHeightMapUpdate->seen = false;

    mGlWidget->update();
  }

private:
  QOpenGLWidget* mGlWidget = nullptr;

  std::shared_ptr<HeightMapUpdate> mHeightMapUpdate;
};

class GlWidget final : public QOpenGLWidget
{
public:
  GlWidget(QWidget* parent)
    : QOpenGLWidget(parent)
    , mHeightMapUpdate(new HeightMapUpdate())
  {}

  auto MakeHeightMapUpdater() -> std::unique_ptr<HeightMapObserver>
  {
    using Ret = std::unique_ptr<HeightMapObserver>;

    return Ret(new HeightMapUpdater(this, mHeightMapUpdate));
  }

private:
  void initializeGL() override
  {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    mShader = std::make_unique<Shader>();
  }

  void paintGL() override
  {
    if (!mHeightMapUpdate->seen) {

      UpdateHeightMap();

      mHeightMapUpdate->seen = true;
    }

    if (mShader)
      mShader->Render();
  }

  void resizeGL(int w, int h) override { glViewport(0, 0, w, h); }

  void UpdateHeightMap()
  {
    auto data = mHeightMapUpdate->data;
    auto w = mHeightMapUpdate->w;
    auto h = mHeightMapUpdate->h;

    mShader->UpdateBufferSize(w, h);

    mShader->UpdateHeightMap(data, w, h);
  }

private:
  Camera mCamera;

  std::unique_ptr<Shader> mShader;

  std::shared_ptr<HeightMapUpdate> mHeightMapUpdate;
};

class SceneViewImpl final : public SceneView
{
public:
  SceneViewImpl(QWidget* parent)
    : mGlWidget(parent)
  {
    mGlWidget.setMinimumSize(320, 240);
  }

  QWidget* GetWidget() override { return &mGlWidget; }

  auto MakeHeightMapUpdater() -> std::unique_ptr<HeightMapObserver> override
  {
    return mGlWidget.MakeHeightMapUpdater();
  }

private:
  GlWidget mGlWidget;
};

} // namespace

auto
SceneView::Make(QWidget* parent) -> std::shared_ptr<SceneView>
{
  return std::shared_ptr<SceneView>(new SceneViewImpl(parent));
}
