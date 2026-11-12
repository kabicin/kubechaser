function(add_kubechaser_core_test test_name source_file)
    add_executable(${test_name} ${source_file})
    target_compile_features(${test_name} PRIVATE cxx_std_17)
    target_link_libraries(${test_name} PRIVATE kubechaser::core kubechaser::warnings)
    add_test(NAME ${test_name} COMMAND ${test_name})
endfunction()

add_kubechaser_core_test(logger_test logger_test.cpp)
add_kubechaser_core_test(linalg_test linalg_test.cpp)
add_kubechaser_core_test(reader_test reader_test.cpp)
add_kubechaser_core_test(drawattributes_test drawattributes_test.cpp)
add_kubechaser_core_test(json_test json_test.cpp)

target_compile_definitions(reader_test PRIVATE
    KUBECHASER_TEST_FIXTURE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/fixtures"
)
