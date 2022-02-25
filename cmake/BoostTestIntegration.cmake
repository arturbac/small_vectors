
function(add_boost_test SOURCE_FILE_NAME DEPENDENCY_LIB)
    get_filename_component(TEST_EXECUTABLE_NAME ${SOURCE_FILE_NAME} NAME_WE)

    add_executable(${TEST_EXECUTABLE_NAME} ${SOURCE_FILE_NAME})
    target_link_libraries(${TEST_EXECUTABLE_NAME} 
                          ${DEPENDENCY_LIB}
                          Boost::unit_test_framework )
    target_compile_options(${TEST_EXECUTABLE_NAME} PUBLIC -Wno-disabled-macro-expansion -Wno-used-but-marked-unused -Wno-global-constructors )
    file(READ "${SOURCE_FILE_NAME}" SOURCE_FILE_CONTENTS)
    string(REGEX MATCHALL "BOOST_AUTO_TEST_CASE\\( *([A-Za-z_0-9]+) *\\)" FOUND_TESTS ${SOURCE_FILE_CONTENTS})
    foreach(HIT ${FOUND_TESTS})
        
        string(REGEX REPLACE ".*\\( *([A-Za-z_0-9]+) *\\).*" "\\1" TEST_NAME ${HIT})
#         message(STATUS "Test " ${TEST_NAME} )
        add_test(NAME "${TEST_EXECUTABLE_NAME}.${TEST_NAME}" 
                 COMMAND ${TEST_EXECUTABLE_NAME}
                 "--run_test=${TEST_NAME}" "--catch_system_error=yes" "--detect_memory_leaks=1")
    endforeach()
    
    string(REGEX MATCHALL "BOOST_AUTO_TEST_CASE_TEMPLATE\\( *([A-Za-z_0-9]+) *\\," FOUND_TESTS ${SOURCE_FILE_CONTENTS})
    foreach(HIT ${FOUND_TESTS})
        
        string(REGEX REPLACE ".*\\( *([A-Za-z_0-9]+) *\\,.*" "\\1" TEST_NAME ${HIT})
#         message(STATUS "Test " ${TEST_NAME} )
        add_test(NAME "${TEST_EXECUTABLE_NAME}.${TEST_NAME}" 
                 COMMAND ${TEST_EXECUTABLE_NAME}
                 "--run_test=${TEST_NAME}*" "--catch_system_error=yes" "--detect_memory_leaks=1")
    endforeach()
    
    #
    
endfunction()


function(target_add_boost_test target_name )
  get_target_property( source_files ${target_name} SOURCES )
  
  foreach(source_file ${source_files})
    message( STATUS "${target_name} has ${source_file}" )
    file(READ "${source_file}" source_file_contents)
    string(REGEX MATCHALL "BOOST_AUTO_TEST_CASE\\( *([A-Za-z_0-9]+) *\\)" found_tests ${source_file_contents})
    foreach(found_test ${found_tests})
        string(REGEX REPLACE ".*\\( *([A-Za-z_0-9]+) *\\).*" "\\1" found_test_name ${found_test})
        message(STATUS "Test " ${found_test_name} )
        get_filename_component(source_file_base ${source_file} NAME_WE)
        add_test(NAME "${target_name}.${source_file_base}.${found_test_name}"
                 COMMAND ${target_name} "--run_test=*/${found_test_name}" "--detect_memory_leaks=1" "--catch_system_error=yes")
    endforeach()
    
    string(REGEX MATCHALL "BOOST_AUTO_TEST_CASE\\( *([A-Za-z_0-9]+) *\\," found_tests ${source_file_contents})
    foreach(found_test ${found_tests})
        string(REGEX REPLACE ".*\\( *([A-Za-z_0-9]+) *\\,.*" "\\1" found_test_name ${found_test})
        message(STATUS "Test " ${found_test_name} )
        get_filename_component(source_file_base ${source_file} NAME_WE)
        add_test(NAME "${target_name}.${source_file_base}.${found_test_name}"
                 COMMAND ${target_name} "--run_test=*/${found_test_name}" "--detect_memory_leaks=1" "--catch_system_error=yes")
    endforeach()
  
    string(REGEX MATCHALL "BOOST_AUTO_TEST_CASE_TEMPLATE\\( *([A-Za-z_0-9]+) *\\," found_tests ${source_file_contents})
    foreach(found_test ${found_tests})
        string(REGEX REPLACE ".*\\( *([A-Za-z_0-9]+) *\\,.*" "\\1" found_test_name ${found_test})
        message(STATUS "Test " ${found_test_name} )
        get_filename_component(source_file_base ${source_file} NAME_WE)
        add_test(NAME "${target_name}.${source_file_base}.${found_test_name}"
                 COMMAND ${target_name} "--run_test=*/${found_test_name}*" "--detect_memory_leaks=1" "--catch_system_error=yes")
    endforeach()
  endforeach()
endfunction()
