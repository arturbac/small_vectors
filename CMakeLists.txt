cmake_minimum_required(VERSION 3.21 FATAL_ERROR)

include(cmake/extract_version.cmake)
project(
  small_vectors
  VERSION ${small_vectors_version}
  LANGUAGES CXX
  HOMEPAGE_URL "https://github.com/arturbac/small_vectors")
cmake_policy(
  SET
  CMP0167
  NEW)
cmake_policy(
  SET
  CMP0175
  NEW)

include(CheckCXXCompilerFlag)
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

if(PROJECT_IS_TOP_LEVEL)
  include(FeatureSummary)
endif()
if(PROJECT_IS_TOP_LEVEL)
  message(STATUS "small_vectors v${small_vectors_version}")
endif()
if(PROJECT_SOURCE_DIR
   STREQUAL
   PROJECT_BINARY_DIR)
  message(
    FATAL_ERROR
      "In-source builds not allowed. Please make a new directory (called a build directory) and run CMake from there.")
endif()

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

set(SMALL_VECTORS_EXPECTED_VTABLE_INSTANTATION OFF)

# -----------------------------------------------------------------------------
# options
# -----------------------------------------------------------------------------
if(PROJECT_IS_TOP_LEVEL)
  option(
    SMALL_VECTORS_ENABLE_UNIT_TESTS
    "unit tests available from CTest"
    ON)
  add_feature_info(
    "SMALL_VECTORS_ENABLE_UNIT_TESTS"
    SMALL_VECTORS_ENABLE_UNIT_TESTS
    "unit test available from CTest")
else()
  set(SMALL_VECTORS_ENABLE_UNIT_TESTS OFF)
endif()

if(NOT
   DEFINED
   CMAKE_CXX_STANDARD
   OR CMAKE_CXX_STANDARD
      LESS
      23)
  set(CMAKE_CXX_STANDARD 23)
  set(CMAKE_CXX_STANDARD_REQUIRED ON)
  set(CMAKE_CXX_EXTENSIONS OFF)
endif()

option(
  SMALL_VECTORS_EXPORT_CMAKE_TARGETS
  "Enable cmake targets"
  ON)

add_library(small_vectors)
target_compile_features(small_vectors PUBLIC cxx_std_23)
target_sources(
  small_vectors
  PRIVATE source/safe_buffers.cc
  PUBLIC FILE_SET
         HEADERS
         BASE_DIRS
         $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
         $<INSTALL_INTERFACE:include>)

if(SMALL_VECTORS_EXPECTED_VTABLE_INSTANTATION)
  target_compile_definitions(small_vectors PUBLIC SMALL_VECTORS_EXPECTED_VTABLE_INSTANTATION=1)
  target_sources(small_vectors PRIVATE source/expected.cc)
endif()

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/ DESTINATION include)
install(
  TARGETS small_vectors
  EXPORT small_vectors_targets
  INCLUDES
  DESTINATION include
  FILE_SET HEADERS)

configure_package_config_file(cmake/small_vectorsConfig.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/small_vectorsConfig.cmake
                              INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/small_vectors)
write_basic_package_version_file(
  ${CMAKE_CURRENT_BINARY_DIR}/small_vectorsConfigVersion.cmake
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY SameMajorVersion)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/small_vectorsConfig.cmake
              ${CMAKE_CURRENT_BINARY_DIR}/small_vectorsConfigVersion.cmake
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/small_vectors)

if(SMALL_VECTORS_EXPORT_CMAKE_TARGETS)
  install(
    EXPORT small_vectors_targets
    FILE small_vectorsTargets.cmake
    NAMESPACE small_vectors::
    DESTINATION lib/cmake/small_vectors)
endif()

if(SMALL_VECTORS_ENABLE_UNIT_TESTS AND PROJECT_IS_TOP_LEVEL)
  enable_testing()
  add_subdirectory(unit_tests)
endif()

if(PROJECT_IS_TOP_LEVEL)
  feature_summary(WHAT ALL)
endif()
