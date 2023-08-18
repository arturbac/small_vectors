cmake_minimum_required(VERSION 3.20 FATAL_ERROR )
include(CheckCXXCompilerFlag)

project(small_vectors
        VERSION 2.2.1
        LANGUAGES CXX
        HOMEPAGE_URL "https://github.com/arturbac/small_vectors"
        ) 

if(PROJECT_SOURCE_DIR STREQUAL PROJECT_BINARY_DIR)
  message(
    FATAL_ERROR
      "In-source builds not allowed. Please make a new directory (called a build directory) and run CMake from there."
  )
endif()

set(CMAKE_MODULE_PATH
  ${PROJECT_SOURCE_DIR}/cmake
  )
  
include(FeatureSummary)
option(SMALL_VECTORS_ENABLE_CPM_PACKAGING "cmake packaging by cpm" ON)

if(SMALL_VECTORS_ENABLE_CPM_PACKAGING)
  include(cmake/CPM.cmake)

  # ---- Add dependencies via CPM ----
  # see https://github.com/cpm-cmake/CPM.cmake for more info

  CPMAddPackage(
    NAME PackageProject.cmake
    GITHUB_REPOSITORY TheLartians/PackageProject.cmake
    VERSION 1.11.0
  )
endif()
#-----------------------------------------------------------------------------
# options
#-----------------------------------------------------------------------------
if(SMALL_VECTORS_ENABLE_CPM_PACKAGING)
  option(SMALL_VECTORS_ENABLE_UNIT_TESTS "unit tests available from CTest"  ${PROJECT_IS_TOP_LEVEL} )
  add_feature_info("SMALL_VECTORS_ENABLE_UNIT_TESTS" SMALL_VECTORS_ENABLE_UNIT_TESTS "unit test available from CTest")
else()
  set(SMALL_VECTORS_ENABLE_UNIT_TESTS OFF)
endif()

if(SMALL_VECTORS_ENABLE_UNIT_TESTS)
  #----------------------------------------------------------------
  # boost-ext/ut
  #----------------------------------------------------------------
  CPMAddPackage(
    ut
    GITHUB_REPOSITORY boost-ext/ut
    GIT_TAG        v1.1.9
  )
endif()

if(NOT DEFINED CMAKE_CXX_STANDARD OR CMAKE_CXX_STANDARD LESS 20 )
  message(STATUS "Assuming by default c++20 standard")
  set( CMAKE_CXX_STANDARD 20 )
endif()
set( CMAKE_CXX_STANDARD_REQUIRED ON )
set( CMAKE_CXX_EXTENSIONS OFF)

message(STATUS "CMake install directory: " ${CMAKE_INSTALL_INCLUDEDIR})

add_library(small_vectors INTERFACE)
if(NOT DEFINED INCLUDE_INSTALL_DIR)
  set(INCLUDE_INSTALL_DIR include/${PROJECT_NAME})
endif()

target_include_directories(small_vectors 
  INTERFACE   $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
              $<INSTALL_INTERFACE:${INCLUDE_INSTALL_DIR}>
)

target_compile_features(small_vectors INTERFACE cxx_std_20)
if( PROJECT_IS_TOP_LEVEL )
  if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
      option(SMALL_VECTORS_ENABLE_LLD_LINKER "enable lld linker for linking unit tests" ON )
      add_feature_info("SMALL_VECTORS_ENABLE_LLD_LINKER" SMALL_VECTORS_ENABLE_LLD_LINKER "enable lld linker for linking unit tests")
      
      set(SMALL_VECTORS_COMPILE_OPTIONS
        -Weverything
        -Werror
        -Wno-c++98-compat
        -Wno-c++98-compat-pedantic
        -Wno-pre-c++14-compat
        -Wno-pre-c++17-compat
        -Wno-pre-c++20-compat-pedantic
        -Wno-c++20-compat
        -Wno-unused-parameter
        -Wno-padded
        -Wno-unused-command-line-argument
        )
      check_cxx_compiler_flag(-Wunsafe-buffer-usage WUNSAFE_BUFFER_USAGE)
      if(WUNSAFE_BUFFER_USAGE)
        list(APPEND SMALL_VECTORS_COMPILE_OPTIONS -Wno-unsafe-buffer-usage)
      endif()
      if(SMALL_VECTORS_ENABLE_LLD_LINKER)
        add_link_options( -fuse-ld=lld )
      endif()
  endif()
  if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
      set(SMALL_VECTORS_COMPILE_OPTIONS
        -Wall
        -Wextra
        -Werror
      )
  endif()
endif()

if(SMALL_VECTORS_ENABLE_CPM_PACKAGING)
  packageProject(
    NAME ${PROJECT_NAME}
    VERSION ${PROJECT_VERSION}
    NAMESPACE small_vectors
    BINARY_DIR ${PROJECT_BINARY_DIR}
    INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include
    INCLUDE_DESTINATION ${INCLUDE_INSTALL_DIR}
    COMPATIBILITY SameMajorVersion
  )
endif()

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/ DESTINATION ${INCLUDE_INSTALL_DIR} )

if(SMALL_VECTORS_ENABLE_UNIT_TESTS)
  add_custom_target( unit_tests )
endif()

if( SMALL_VECTORS_ENABLE_UNIT_TESTS )
  enable_testing( TRUE )
  add_subdirectory(unit_tests)
endif()

if( PROJECT_IS_TOP_LEVEL )
  feature_summary(WHAT ALL)
endif()
