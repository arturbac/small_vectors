cmake_minimum_required(VERSION 3.19 FATAL_ERROR )

project(fixed_math
        LANGUAGES CXX )

set(CMAKE_MODULE_PATH
  ${PROJECT_SOURCE_DIR}/cmake
  )
  
include(FeatureSummary)
include(GNUInstallDirs)
include(BoostTestIntegration)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

#-----------------------------------------------------------------------------
# options
#-----------------------------------------------------------------------------
option(ENABLE_UNIT_TESTS "unit tests available from CTest" ON )

add_feature_info("ENABLE_UNIT_TESTS" ENABLE_UNIT_TESTS "unit test available from CTest")

if(NOT DEFINED CMAKE_CXX_STANDARD OR CMAKE_CXX_STANDARD LESS 20 )
  message(STATUS "Assuming by default c++20 standard")
  set( CMAKE_CXX_STANDARD 20 )
endif()
set( CMAKE_CXX_STANDARD_REQUIRED ON )
set( CMAKE_CXX_EXTENSIONS OFF)

add_library(small_vectors INTERFACE)
target_include_directories(small_vectors INTERFACE include )

if(ENABLE_UNIT_TESTS)
  find_package( Boost 1.78 REQUIRED COMPONENTS unit_test_framework system )
  if(Boost_FOUND)
    message(" -- Found boost " ${Boost_VERSION})
    message("    : Includes " ${Boost_INCLUDE_DIRS})
    message("    : Libs     " ${Boost_LIBRARIES})
  endif()
  add_custom_target( unit_tests )
endif()

if( ENABLE_UNIT_TESTS )
  enable_testing( TRUE )
  add_subdirectory(unit_tests)
endif()


feature_summary(WHAT ALL)
