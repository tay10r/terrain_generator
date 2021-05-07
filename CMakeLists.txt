cmake_minimum_required(VERSION 3.14.7)

project(mapgen)

find_package(Qt5 REQUIRED COMPONENTS Widgets)

find_package(OpenGL REQUIRED COMPONENTS OpenGL)

include(FetchContent)

FetchContent_Declare(nodeeditor
  URL "https://github.com/paceholder/nodeeditor/archive/master.zip")

FetchContent_MakeAvailable(nodeeditor)

FetchContent_Declare(glm
  URL "https://github.com/g-truc/glm/archive/master.zip")

FetchContent_MakeAvailable(glm)

add_library(mapgen
  Backend.h
  Backend.cpp
  CpuBackend.h
  CpuBackend.cpp
  IR.h
  IR.cpp)

target_include_directories(mapgen PUBLIC "${PROJECT_SOURCE_DIR}")

target_compile_features(mapgen PUBLIC cxx_std_17)

target_link_libraries(mapgen PUBLIC glm OpenGL::OpenGL)

set(CMAKE_AUTOMOC ON)

add_executable(mapgen_gui main.cpp)

set_target_properties(mapgen_gui
  PROPERTIES
    OUTPUT_NAME mapgen)

target_link_libraries(mapgen_gui PRIVATE Qt5::Widgets NodeEditor::nodes mapgen)

if(NOT MSVC)

  target_compile_options(mapgen PRIVATE -Wall -Wextra -Werror -Wfatal-errors)

  target_compile_options(mapgen_gui PRIVATE -Wall -Wextra -Werror -Wfatal-errors)

endif(NOT MSVC)

add_subdirectory(tests)

enable_testing()
