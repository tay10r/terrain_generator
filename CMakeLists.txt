cmake_minimum_required(VERSION 3.14.7)

project(mapgen)

find_package(Qt5 REQUIRED COMPONENTS Widgets)

find_package(OpenGL REQUIRED COMPONENTS OpenGL)

include(FetchContent)

FetchContent_Declare(nodeeditor
  URL "https://github.com/tay10r/nodeeditor/archive/master.zip")

FetchContent_MakeAvailable(nodeeditor)

FetchContent_Declare(glm
  URL "https://github.com/g-truc/glm/archive/master.zip")

FetchContent_MakeAvailable(glm)

add_library(mapgen
  core/Backend.h
  core/Backend.cpp
  core/Camera.h
  core/Camera.cpp
  core/CpuBackend.h
  core/CpuBackend.cpp
  core/IR.h
  core/IR.cpp)

target_include_directories(mapgen PUBLIC "${PROJECT_SOURCE_DIR}")

target_compile_features(mapgen PUBLIC cxx_std_17)

target_link_libraries(mapgen PUBLIC glm OpenGL::OpenGL)

add_executable(mapgen_gui
  main.cpp
  gui/BackendUpdater.h
  gui/BackendUpdater.cpp
  gui/CoordinatesModel.h
  gui/CoordinatesModel.cpp
  gui/ConstantsModels.h
  gui/ConstantsModels.cpp
  gui/ExprNodeData.h
  gui/ExprNodeData.cpp
  gui/Editor.h
  gui/Editor.cpp
  gui/MenuBar.h
  gui/MenuBar.cpp
  gui/OutputModels.h
  gui/OutputModels.cpp
  gui/SceneView.h
  gui/SceneView.cpp
  gui/Workspace.h
  gui/Workspace.cpp)

set_target_properties(mapgen_gui
  PROPERTIES
    OUTPUT_NAME mapgen
    AUTOMOC ON)

target_link_libraries(mapgen_gui PRIVATE Qt5::Widgets NodeEditor::nodes mapgen)

if(NOT MSVC)

  target_compile_options(mapgen PRIVATE -Wall -Wextra -Werror -Wfatal-errors)

  target_compile_options(mapgen_gui PRIVATE -Wall -Wextra -Werror -Wfatal-errors)

endif(NOT MSVC)

add_subdirectory(tests)

enable_testing()
