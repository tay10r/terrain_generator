cmake_minimum_required(VERSION 3.9.6)

project(terra_lib)

add_executable(glsl2h utils/glsl2h.cpp)

find_program(GLSLANG_VALIDATOR glslangValidator)

if(GLSLANG_VALIDATOR)
  add_custom_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/shaders.cpp"
    COMMAND ${GLSLANG_VALIDATOR} "${CMAKE_CURRENT_SOURCE_DIR}/shaders/render_tile.vert"
    COMMAND ${GLSLANG_VALIDATOR} "${CMAKE_CURRENT_SOURCE_DIR}/shaders/render_tile.frag"
    COMMAND $<TARGET_FILE:glsl2h>
      "${CMAKE_CURRENT_SOURCE_DIR}/shaders/render_tile.vert"
      "${CMAKE_CURRENT_SOURCE_DIR}/shaders/render_tile.frag"
      "${CMAKE_CURRENT_SOURCE_DIR}/src/shaders.cpp.in"
      "${CMAKE_CURRENT_BINARY_DIR}/shaders.cpp"
    DEPENDS
      "${CMAKE_CURRENT_SOURCE_DIR}/shaders/render_tile.vert"
      "${CMAKE_CURRENT_SOURCE_DIR}/shaders/render_tile.frag")
else(GLSLANG_VALIDATOR)
  add_custom_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/shaders.cpp"
    COMMAND $<TARGET_FILE:glsl2h>
      "${CMAKE_CURRENT_SOURCE_DIR}/shaders/render_tile.vert"
      "${CMAKE_CURRENT_SOURCE_DIR}/shaders/render_tile.frag"
      "${CMAKE_CURRENT_SOURCE_DIR}/src/shaders.cpp.in"
      "${CMAKE_CURRENT_BINARY_DIR}/shaders.cpp"
    DEPENDS
      "${CMAKE_CURRENT_SOURCE_DIR}/shaders/render_tile.vert"
      "${CMAKE_CURRENT_SOURCE_DIR}/shaders/render_tile.frag")
endif(GLSLANG_VALIDATOR)

find_package(PNG REQUIRED)
find_package(Eigen3 REQUIRED)

include(FetchContent)

FetchContent_Declare(thread_pool
  URL "https://github.com/bshoshany/thread-pool/archive/master.zip")

FetchContent_MakeAvailable(thread_pool)

set(incdir "${CMAKE_CURRENT_SOURCE_DIR}/include/terra")
set(srcdir "${CMAKE_CURRENT_SOURCE_DIR}/src")

add_library(terra
  "${incdir}/tile.h"
  "${srcdir}/tile.cpp"
  "${incdir}/tile_observer.h"
  "${incdir}/png_writer.h"
  "${srcdir}/png_writer.cpp"
  "${incdir}/interpreter.h"
  "${srcdir}/interpreter.cpp"
  "${incdir}/type.h"
  "${incdir}/expr.h"
  "${incdir}/expr_visitor.h"
  "${incdir}/exprs/var_ref.h"
  "${srcdir}/exprs/var_ref.cpp"
  "${incdir}/exprs/literals.h"
  "${incdir}/exprs/unary.h"
  "${srcdir}/exprs/unary.cpp"
  "${srcdir}/shaders.h"
  "${CMAKE_CURRENT_BINARY_DIR}/shaders.cpp")

target_link_libraries(terra PUBLIC PNG::PNG Eigen3::Eigen)

target_include_directories(terra
  PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include"
  PRIVATE
    ${thread_pool_SOURCE_DIR}
    "${CMAKE_CURRENT_SOURCE_DIR}/src")

target_compile_features(terra PUBLIC cxx_std_17)

add_executable(test-export
  test-export.cpp)

target_link_libraries(test-export PRIVATE terra)

find_package(Qt5 REQUIRED COMPONENTS Widgets)

add_executable(test-view test-view.cpp)

target_link_libraries(test-view PRIVATE terra Qt::Widgets)

if(NOT MSVC)

  set(cxxflags -Wall -Wextra -Werror -Wfatal-errors)

endif(NOT MSVC)

target_compile_options(terra PRIVATE ${cxxflags})

target_compile_options(test-export PRIVATE ${cxxflags})

target_compile_options(test-view PRIVATE ${cxxflags})
