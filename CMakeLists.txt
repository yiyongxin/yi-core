cmake_minimum_required(VERSION 3.24)
project(yi_core VERSION 0.0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Library target
add_library(yi_core STATIC
    src/placeholder.cpp
)

target_include_directories(yi_core PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

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

install(TARGETS yi_core EXPORT YiCoreTargets)
install(EXPORT YiCoreTargets FILE YiCoreTargets.cmake DESTINATION lib/cmake/yi_core)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/YiCoreConfig.cmake" "${CMAKE_CURRENT_BINARY_DIR}/YiCoreConfigVersion.cmake" DESTINATION lib/cmake/yi_core)

# Enable tests only when built as top-level project
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
  include(CTest)
  enable_testing()
  add_subdirectory(tests)
endif()
