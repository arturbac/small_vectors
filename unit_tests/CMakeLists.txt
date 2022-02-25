add_library( ut_core )
target_sources( ut_core PRIVATE unit_test_core.cc )
target_link_libraries( ut_core PUBLIC small_vectors Boost::unit_test_framework Boost::system )
target_include_directories( ut_core PUBLIC ut_core )
target_compile_options(ut_core PUBLIC -Wno-disabled-macro-expansion -Wno-used-but-marked-unused -Wno-global-constructors )

add_boost_test(uninitialize_constexpr_ut.cc ut_core)
add_dependencies(unit_tests uninitialize_constexpr_ut )

add_boost_test(static_vector_ut.cc ut_core)
add_dependencies(unit_tests static_vector_ut )
#target_compile_options( static_vector_ut PRIVATE "-Wsign-conversion" )

add_boost_test(small_vector_ut.cc ut_core)
add_dependencies(unit_tests small_vector_ut )
#target_compile_options( small_vector_ut PRIVATE "-Wsign-conversion" )

add_boost_test(composed_pointer_with_data_ut.cc ut_core)
add_dependencies(unit_tests composed_pointer_with_data_ut )

add_boost_test(unaligned_ut.cc ut_core)
add_dependencies(unit_tests unaligned_ut )
