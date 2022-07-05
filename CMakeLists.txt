cmake_minimum_required(VERSION 3.19 FATAL_ERROR )

project(small_vectors
        VERSION 1.0.1
        LANGUAGES CXX
        HOMEPAGE_URL "https://github.com/arturbac/small_vectors"
        )

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
option(SMALL_VECTORS_ENABLE_UNIT_TESTS "unit tests available from CTest" OFF )

add_feature_info("SMALL_VECTORS_ENABLE_UNIT_TESTS" SMALL_VECTORS_ENABLE_UNIT_TESTS "unit test available from CTest")

if(NOT DEFINED CMAKE_CXX_STANDARD OR CMAKE_CXX_STANDARD LESS 20 )
  message(STATUS "Assuming by default c++20 standard")
  set( CMAKE_CXX_STANDARD 20 )
endif()
set( CMAKE_CXX_STANDARD_REQUIRED ON )
set( CMAKE_CXX_EXTENSIONS OFF)

message(STATUS "CMake inatall directory: " ${CMAKE_INSTALL_INCLUDEDIR})

add_library(small_vectors INTERFACE)
#target_include_directories(small_vectors INTERFACE include )

target_include_directories(small_vectors 
  INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
            $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

target_compile_features(small_vectors INTERFACE cxx_std_20)


include(CMakePackageConfigHelpers)
write_basic_package_version_file("small_vectorsConfigVersion.cmake"
                                 VERSION ${PROJECT_VERSION}
                                 COMPATIBILITY SameMajorVersion)

configure_package_config_file(
  "${PROJECT_SOURCE_DIR}/cmake/${PROJECT_NAME}Config.cmake.in"
  "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
  INSTALL_DESTINATION
  ${CMAKE_INSTALL_DATAROOTDIR}/${PROJECT_NAME}/cmake
)
install(TARGETS small_vectors
        EXPORT ${PROJECT_NAME}_Targets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

install(EXPORT ${PROJECT_NAME}_Targets
        FILE ${PROJECT_NAME}Targets.cmake
        NAMESPACE ${PROJECT_NAME}::
        DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/${PROJECT_NAME}/cmake)

install(FILES "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
              "${PROJECT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
        DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/${PROJECT_NAME}/cmake)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include DESTINATION include/small_vectors)

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
