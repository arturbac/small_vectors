
function( add_unittest name )
  add_executable(${name})
  target_sources(${name} PRIVATE ${name}.cc)
  target_link_libraries( ${name} PRIVATE small_vectors_ut_core )

  add_dependencies(unit_tests ${name} )
  add_test( NAME ${name}_test COMMAND ${name} )
endfunction()

