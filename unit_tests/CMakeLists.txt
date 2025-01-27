include(${PROJECT_SOURCE_DIR}/cmake/get_cpm.cmake)
if(CMAKE_CXX_COMPILER_ID
   STREQUAL
   "Clang")
  option(
    SMALL_VECTORS_ENABLE_LLD_LINKER
    "enable lld linker for linking unit tests"
    ON)
  add_feature_info(
    "SMALL_VECTORS_ENABLE_LLD_LINKER"
    SMALL_VECTORS_ENABLE_LLD_LINKER
    "enable lld linker for linking unit tests")

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
      -Wno-unused-command-line-argument)
  check_cxx_compiler_flag(-Wunsafe-buffer-usage WUNSAFE_BUFFER_USAGE)
  if(WUNSAFE_BUFFER_USAGE)
    list(
      APPEND
      SMALL_VECTORS_COMPILE_OPTIONS
      -Wunsafe-buffer-usage)
  endif()
  check_cxx_compiler_flag(-Wswitch-default WNO_SWITCH_DEFAULT)
  if(WNO_SWITCH_DEFAULT)
    list(
      APPEND
      SMALL_VECTORS_COMPILE_OPTIONS
      -Wno-switch-default)
  endif()

  if(SMALL_VECTORS_ENABLE_LLD_LINKER)
    add_link_options(-fuse-ld=lld)
  endif()
endif()
if(CMAKE_CXX_COMPILER_ID
   STREQUAL
   "GNU")
  set(SMALL_VECTORS_COMPILE_OPTIONS
      -Wall
      -Wextra
      -Werror)
endif()

# ----------------------------------------------------------------
# boost-ext/ut
# ----------------------------------------------------------------
cpmaddpackage(
  ut
  GITHUB_REPOSITORY
  arturbac/ut-ext
  GIT_TAG
  master)
add_custom_target(unit_tests)
add_library(small_vectors_ut_core)
target_sources(small_vectors_ut_core PRIVATE unit_test_core.cc)
target_link_libraries(small_vectors_ut_core PUBLIC small_vectors Boost::ut)
target_include_directories(small_vectors_ut_core PUBLIC ut_core)
target_compile_options(small_vectors_ut_core PUBLIC ${SMALL_VECTORS_COMPILE_OPTIONS})
if(CMAKE_CXX_COMPILER_ID
   STREQUAL
   "Clang"
   OR CMAKE_CXX_COMPILER_ID
      STREQUAL
      "AppleClang")
  target_compile_options(
    small_vectors_ut_core
    PUBLIC -Wno-disabled-macro-expansion
           -Wno-used-but-marked-unused
           -Wno-global-constructors
           -Wno-exit-time-destructors
           -Wno-ctad-maybe-unsupported
           -Wno-weak-vtables
           -fconstexpr-backtrace-limit=0
           -Wno-misleading-indentation)
endif()
if(CMAKE_CXX_COMPILER_ID
   STREQUAL
   "GNU")
  target_compile_options(
    small_vectors_ut_core
    PUBLIC -Wno-misleading-indentation
           -Wno-attributes
           -Wno-unused-parameter
           -Wno-unknown-pragmas)
endif()

include(small_vectors_ut)
set(COMPILER_ID "${CMAKE_CXX_COMPILER_ID}")
set(COMPILER_VERSION "${CMAKE_CXX_COMPILER_VERSION}")
set(COMPILER_INFO "${COMPILER_ID} ${COMPILER_VERSION}")
message(STATUS "Compiler Info: ${COMPILER_INFO}")

get_filename_component(
  LAST_PATH_ELEMENT
  "${CMAKE_BINARY_DIR}"
  NAME)
set(COMPILER_INFO "${COMPILER_INFO}_at_${LAST_PATH_ELEMENT}")
message(STATUS "Compiler and Standard Library Info: ${COMPILER_INFO}")

add_unittest(unaligned_ut)
target_compile_options(unaligned_ut PRIVATE -Wno-float-equal)

add_unittest(uninitialized_constexpr_ut)
add_unittest(static_vector_ut)
target_compile_options(static_vector_ut PRIVATE -Wno-float-equal)

add_unittest(small_vector_ut)
add_unittest(composed_pointer_with_data_ut)
add_unittest(meta_packed_struct_ut)
add_unittest(strong_type_ut)
add_unittest(string_ut)
add_unittest(ranges_ut)
add_unittest(expected_ut)
add_unittest(inclass_storage_ut)

# github ubuntu latest is very old
find_package(Boost 1.74 COMPONENTS system)
if(Boost_FOUND)
  add_unittest(bwt_ut)
  target_link_libraries(bwt_ut PRIVATE Boost::system)

  add_unittest(shared_mem_util_ut)
  target_link_libraries(shared_mem_util_ut PRIVATE Boost::system)
  target_compile_definitions(shared_mem_util_ut PRIVATE SMALL_VECTORS_COMPILER_INFO="${COMPILER_INFO}")
  # add_unittest(ring_queue_ut) target_link_libraries(ring_queue_ut PRIVATE Boost::system )

  # add_unittest(stack_buffer_ut) target_link_libraries(stack_buffer_ut PRIVATE Boost::system )
  # target_compile_definitions(stack_buffer_ut PRIVATE SMALL_VECTORS_COMPILER_INFO="${COMPILER_INFO}")
endif()
