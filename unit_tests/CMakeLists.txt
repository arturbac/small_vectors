add_library( small_vectors_ut_core )
target_sources( small_vectors_ut_core PRIVATE unit_test_core.cc )
target_link_libraries( small_vectors_ut_core
  PUBLIC
    small_vectors
    Boost::ut
    )
target_include_directories( small_vectors_ut_core PUBLIC ut_core )
target_compile_options(small_vectors_ut_core
  PUBLIC
    ${SMALL_VECTORS_COMPILE_OPTIONS}
    )
if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  target_compile_options(small_vectors_ut_core
    PUBLIC
      -Wno-disabled-macro-expansion
      -Wno-used-but-marked-unused
      -Wno-global-constructors
      -Wno-exit-time-destructors
      -Wno-ctad-maybe-unsupported
      )
endif()

include(small_vectors_ut)

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
add_unittest(utf_ut)

# github ubuntu latest is very old
find_package( Boost 1.74 COMPONENTS system )
if(Boost_FOUND)
  add_unittest(shared_mem_util_ut)
  target_link_libraries(shared_mem_util_ut PRIVATE Boost::system )
  
  # add_unittest(ring_queue_ut)
  # target_link_libraries(ring_queue_ut PRIVATE Boost::system )
endif()
