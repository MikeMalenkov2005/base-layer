cmake_minimum_required(VERSION 3.16)
project(BaseLayer C)

set(CMAKE_C_STANDARD 99)

option(BASE_LAYER_INCLUDE_GUI "Include GUI in the base layer build" OFF)

if (BASE_LAYER_INCLUDE_GUI)
  add_compile_definitions(BASE_LAYER_INCLUDE_GUI)
endif()

file(GLOB BASE_LAYER_SOURCES "src/*.c")

add_library(${PROJECT_NAME} STATIC ${BASE_LAYER_SOURCES})
target_include_directories(${PROJECT_NAME} PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include> $<INSTALL_INTERFACE:include>)
