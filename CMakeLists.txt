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

set(CMAKE_AUTOMOC ON)

add_executable(mapgen
  main.cpp
  Backend.h
  Backend.cpp
  CpuBackend.h
  CpuBackend.cpp
  IR.h
  IR.cpp)

target_compile_features(mapgen PRIVATE cxx_std_17)

if(NOT MSVC)
  target_compile_options(mapgen PRIVATE -Wall -Wextra -Werror -Wfatal-errors)
endif(NOT MSVC)

target_link_libraries(mapgen PRIVATE Qt5::Widgets NodeEditor::nodes glm OpenGL::OpenGL)
