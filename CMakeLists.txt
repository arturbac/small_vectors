cmake_minimum_required(VERSION 3.20 FATAL_ERROR )

project(small_vectors
        VERSION 1.0.4
        LANGUAGES CXX
        HOMEPAGE_URL "https://github.com/arturbac/small_vectors"
        )

set(CMAKE_MODULE_PATH
  ${PROJECT_SOURCE_DIR}/cmake
  )
  
include(FeatureSummary)
include(GNUInstallDirs)
include(BoostTestIntegration)
include(cmake/CPM.cmake)

# ---- Add dependencies via CPM ----
# see https://github.com/cpm-cmake/CPM.cmake for more info
# PackageProject.cmake will be used to make our target installable
CPMAddPackage(
  NAME PackageProject.cmake
  GITHUB_REPOSITORY TheLartians/PackageProject.cmake
  VERSION 1.9.0
)

#-----------------------------------------------------------------------------
# options
#-----------------------------------------------------------------------------

option(SMALL_VECTORS_ENABLE_UNIT_TESTS "unit tests available from CTest" ${PROJECT_IS_TOP_LEVEL} )

add_feature_info("SMALL_VECTORS_ENABLE_UNIT_TESTS" SMALL_VECTORS_ENABLE_UNIT_TESTS "unit test available from CTest")

if(NOT DEFINED CMAKE_CXX_STANDARD OR CMAKE_CXX_STANDARD LESS 20 )
  message(STATUS "Assuming by default c++20 standard")
  set( CMAKE_CXX_STANDARD 20 )
endif()
set( CMAKE_CXX_STANDARD_REQUIRED ON )
set( CMAKE_CXX_EXTENSIONS OFF)

message(STATUS "CMake inatall directory: " ${CMAKE_INSTALL_INCLUDEDIR})

add_library(small_vectors INTERFACE)
if(NOT DEFINED INCLUDE_INSTALL_DIR)
  set(INCLUDE_INSTALL_DIR include/${PROJECT_NAME})
endif()

target_include_directories(small_vectors 
  INTERFACE   $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
              $<INSTALL_INTERFACE:${INCLUDE_INSTALL_DIR}>
)

target_compile_features(small_vectors INTERFACE cxx_std_20)

packageProject(
  NAME ${PROJECT_NAME}
  VERSION ${PROJECT_VERSION}
  NAMESPACE small_vectors
  BINARY_DIR ${PROJECT_BINARY_DIR}
  INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include
  INCLUDE_DESTINATION ${INCLUDE_INSTALL_DIR}
  COMPATIBILITY SameMajorVersion
)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include DESTINATION ${INCLUDE_INSTALL_DIR} )

if(SMALL_VECTORS_ENABLE_UNIT_TESTS)
  find_package( Boost 1.78 REQUIRED COMPONENTS unit_test_framework system )
  if(Boost_FOUND)
    message(" -- Found boost " ${Boost_VERSION})
    message("    : Includes " ${Boost_INCLUDE_DIRS})
    message("    : Libs     " ${Boost_LIBRARIES})
  endif()
  add_custom_target( unit_tests )
endif()

if( SMALL_VECTORS_ENABLE_UNIT_TESTS )
  enable_testing( TRUE )
  add_subdirectory(unit_tests)
endif()


feature_summary(WHAT ALL)
