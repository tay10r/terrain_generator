#include <terra/interpreter.h>
#include <terra/tile.h>
#include <terra/tile_observer.h>

#include <terra/exprs/literals.h>
#include <terra/exprs/var_ref.h>
#include <terra/exprs/vector_combiner.h>

#include <QApplication>
#include <QDebug>
#include <QMainWindow>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLWidget>
#include <QTimer>

#include "src/shaders.h"

namespace {

class TerrainView final : public QOpenGLWidget
{
public:
  TerrainView(QWidget* parent)
    : QOpenGLWidget(parent)
  {}

  ~TerrainView()
  {
    makeCurrent();

    if (mVertexBuffer)
      context()->functions()->glDeleteBuffers(1, &mVertexBuffer);

    doneCurrent();
  }

  void RenderTile(const terra::Tile& tile)
  {
    makeCurrent();

    UpdateVertexBuffer(tile);

    UpdateElementBuffer(tile);

    auto* funcs = context()->functions();

    funcs->glBindBuffer(GL_ARRAY_BUFFER, mVertexBuffer);

    funcs->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mElementBuffer);

    auto indexCount = (tile.GetWidth() - 1) * (tile.GetHeight() - 1);

    mProgram.bind();

    funcs->glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_SHORT, 0);

    funcs->glBindBuffer(GL_ARRAY_BUFFER, 0);

    funcs->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    doneCurrent();
  }

private:
  void initializeGL() override
  {
    using namespace terra;

    mProgram.addShaderFromSourceCode(QOpenGLShader::Vertex, gVertSource);

    if (mProgram.log().size() > 0)
      qDebug() << mProgram.log();

    mProgram.addShaderFromSourceCode(QOpenGLShader::Fragment, gFragSource);

    if (mProgram.log().size() > 0)
      qDebug() << mProgram.log();

    mProgram.link();

    if (mProgram.log().size() > 0)
      qDebug() << mProgram.log();

    mProgram.setUniformValue("gMVP", QMatrix4x4());

    mProgram.setUniformValue("gNormalMVP", QMatrix3x3());

    mPositionAttr = mProgram.attributeLocation("gPosition");

    mNormalAttr = mProgram.attributeLocation("gNormal");
  }

  void UpdateVertexBuffer(const terra::Tile& tile)
  {
    auto* funcs = context()->functions();

    if (!mVertexBuffer) {

      funcs->glGenBuffers(1, &mVertexBuffer);
    }

    auto vertexData = GenerateVertexData(tile);

    funcs->glBindBuffer(GL_ARRAY_BUFFER, mVertexBuffer);

    funcs->glVertexAttribPointer(mPositionAttr, 3, GL_FLOAT, GL_FALSE, 0, 0);

    auto nrmPtr = (const void*)(tile.GetWidth() * tile.GetHeight() * 3);

    funcs->glVertexAttribPointer(mNormalAttr, 3, GL_FLOAT, GL_FALSE, 0, nrmPtr);

    funcs->glBufferData(GL_ARRAY_BUFFER,
                        vertexData.size() * sizeof(vertexData[0]),
                        vertexData.data(),
                        GL_DYNAMIC_DRAW);

    funcs->glBindBuffer(GL_ARRAY_BUFFER, 0);
  }

  auto GenerateVertexData(const terra::Tile& tile) -> std::vector<float>
  {
    auto w = tile.GetWidth();

    auto h = tile.GetHeight();

    auto vertexCount = w * h;

    auto normalCount = (w - 1) * (h - 1);

    std::vector<float> buffer((vertexCount * 3) + (normalCount * 3));

    tile.ToPositionBuffer(buffer.data(), vertexCount * 3);

    tile.ToNormalBuffer(buffer.data() + vertexCount * 3, normalCount * 3);

    return buffer;
  }

  void UpdateElementBuffer(const terra::Tile& tile)
  {
    static_assert(terra::TileSize() <= 256);

    auto verticesPerLine = tile.GetWidth();

    auto w = tile.GetWidth() - 1;
    auto h = tile.GetHeight() - 1;

    std::vector<uint16_t> indices(w * h * 6);

    auto getVertexIndex = [verticesPerLine](size_t x, size_t y) -> size_t {
      return (y * verticesPerLine) + x;
    };

    for (size_t y = 0; y < h; y++) {

      for (size_t x = 0; x < w; x++) {

        auto offset = ((y * w) + x) * 6;

        indices[offset + 0] = getVertexIndex(x + 0, y + 0);
        indices[offset + 1] = getVertexIndex(x + 0, y + 1);
        indices[offset + 2] = getVertexIndex(x + 1, y + 0);

        indices[offset + 3] = getVertexIndex(x + 0, y + 1);
        indices[offset + 4] = getVertexIndex(x + 1, y + 1);
        indices[offset + 5] = getVertexIndex(x + 1, y + 0);
      }
    }

    auto* funcs = context()->functions();

    if (!mElementBuffer)
      funcs->glGenBuffers(1, &mElementBuffer);

    funcs->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mElementBuffer);

    funcs->glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                        indices.size() * sizeof(indices[0]),
                        indices.data(),
                        GL_DYNAMIC_DRAW);

    funcs->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  }

private:
  GLuint mVertexBuffer = 0;

  GLuint mElementBuffer = 0;

  QOpenGLShaderProgram mProgram;

  int mPositionAttr = -1;

  int mNormalAttr = -1;
};

class TileRenderer final : public terra::TileObserver
{
public:
  TileRenderer(TerrainView& terrainView)
    : mTerrainView(terrainView)
  {}

  void Observe(const terra::Tile& tile) override
  {
    mTerrainView.RenderTile(tile);
  }

private:
  TerrainView& mTerrainView;

  GLuint mVertexBuffer;
};

using SharedExprPtr = std::shared_ptr<terra::Expr>;

/// Used for starting and polling render jobs.
class RenderJobController final : public QObject
{
public:
  RenderJobController(terra::TileInterpreter& tileInterpreter)
    : mTileInterpreter(tileInterpreter)
    , mTimer(this)
  {
    // aim for about 30fps
    mTimer.setInterval(33);

    auto connectionType = Qt::QueuedConnection;

    mTimer.callOnTimeout(this, &RenderJobController::CheckJob, connectionType);
  }

  bool StartJob()
  {
    if (!mTileInterpreter.BeginFrame())
      return false;

    mTimer.start();

    return true;
  }

private:
  void CheckJob()
  {
    if (mTileInterpreter.FrameIsDone()) {
      // shouldn't be reachable, but just in case.
      return;
    }

    mTileInterpreter.PollTiles(0);

    if (mTileInterpreter.TilesRemaining() == 0) {

      mTileInterpreter.EndFrame();

      mTimer.stop();
    }
  }

private:
  terra::TileInterpreter& mTileInterpreter;

  QTimer mTimer;
};

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
main(int argc, char** argv)
{
  QApplication app(argc, argv);

  QMainWindow mainWindow;

  TerrainView terrainView(&mainWindow);

  mainWindow.showMaximized();

  mainWindow.setCentralWidget(&terrainView);

  auto heightExpr = MakeHeightExpr();

  auto colorExpr = MakeColorExpr();

  auto renderer = std::make_shared<TileRenderer>(terrainView);

  auto interpreter = terra::TileInterpreter::Make();

  interpreter->AddTileObserver(std::move(renderer));

  interpreter->SetHeightExpr(*heightExpr);

  // interpreter->SetColorExpr(*colorExpr);

  interpreter->SetResolution(1024, 1024);

  RenderJobController jobController(*interpreter);

  jobController.StartJob();

  return app.exec();
}
