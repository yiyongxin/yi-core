cmake_minimum_required(VERSION 3.14)
project(yi_serialize CXX)

# JSON 序列化器使用 C++20 concepts
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ── 头文件库（INTERFACE，无需编译产物）──────────────────────────────────────
add_library(yi_serialize INTERFACE)
add_library(yi::serialize ALIAS yi_serialize)

target_include_directories(yi_serialize INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
  $<INSTALL_INTERFACE:include>
)

# RapidJSON（JSON 序列化器依赖）
find_path(RAPIDJSON_INCLUDE_DIR rapidjson/document.h
  HINTS /usr/local/include /usr/include
  REQUIRED
)
target_include_directories(yi_serialize INTERFACE ${RAPIDJSON_INCLUDE_DIR})

# ── 单元测试（仅当作为顶层项目构建时）──────────────────────────────────────
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
  include(CTest)
  enable_testing()
  add_subdirectory(tests)
endif()
