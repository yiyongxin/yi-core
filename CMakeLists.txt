cmake_minimum_required(VERSION 3.24)
project(yi_core VERSION 0.0.2 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ------------------------------------------------------------
# yi_core CMake 列表
#
# 说明：
# - 本文件定义核心库 `yi_core` 的构建与导出规则，目的是让 `yi_core`
#   成为一个可发布、可被下游 find_package() 使用的基础库（低频发布）。
# - 公开头文件必须放在 `include/yi/...` 下；实现放在 `src/`。
# - 当作为顶层工程直接构建时，会启用 `tests/` 子目录并运行单元测试；
#   当被下游项目通过 add_subdirectory() 或 find_package() 使用时，tests 不会自动启用。
#
# 典型构建：
# ```bash
# mkdir -p build && cd build
# cmake -DCMAKE_BUILD_TYPE=Release ..
# cmake --build . -j$(nproc)
# ctest --output-on-failure
# ```
#
# 本地联调（覆盖远程依赖）：在下游项目使用 FetchContent 时可设置 SOURCE_DIR 指向本地 yi_core 源码。
# ------------------------------------------------------------

# ------------------------------------------------------------
# 第三方依赖: stdexec (NVIDIA/stdexec — C++26 std::execution 参考实现)
#
# 提供: exec::task<T>、stdexec::run_loop、stdexec::on、stdexec::then 等
# 约束: 仅使用 CPU 路径 (STDEXEC::stdexec)，不引入 CUDA/GPU 相关目标
# ------------------------------------------------------------
include(FetchContent)

set(STDEXEC_BUILD_TESTS    OFF CACHE BOOL "" FORCE)
set(STDEXEC_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    stdexec
    GIT_REPOSITORY https://github.com/NVIDIA/stdexec.git
    GIT_TAG        main
    GIT_SHALLOW    TRUE
)

FetchContent_MakeAvailable(stdexec)

# Library target
add_library(yi_core STATIC
    src/placeholder.cpp
)

target_include_directories(yi_core PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(yi_core PUBLIC STDEXEC::stdexec)

add_library(yi::core ALIAS yi_core)

# Packaging helpers
include(CMakePackageConfigHelpers)

# Export targets and generate config files when installing
write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/YiCoreConfigVersion.cmake"
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY AnyNewerVersion
)

configure_package_config_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake/YiCoreConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/YiCoreConfig.cmake"
  INSTALL_DESTINATION lib/cmake/yi_core
)

option(YI_CORE_INSTALL "Generate install rules for yi_core" OFF)
if(YI_CORE_INSTALL)
    install(TARGETS yi_core EXPORT YiCoreTargets)
    install(EXPORT YiCoreTargets FILE YiCoreTargets.cmake DESTINATION lib/cmake/yi_core)
    install(FILES "${CMAKE_CURRENT_BINARY_DIR}/YiCoreConfig.cmake" "${CMAKE_CURRENT_BINARY_DIR}/YiCoreConfigVersion.cmake" DESTINATION lib/cmake/yi_core)
endif()

# Enable tests only when built as top-level project
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
  include(CTest)
  enable_testing()
  add_subdirectory(tests)
endif()
