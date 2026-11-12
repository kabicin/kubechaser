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

# Tests for the renderer, entities and scene logic. These link the full
# application library, so they need KUBECHASER_BUILD_APP.
if(KUBECHASER_BUILD_APP)
    function(add_kubechaser_app_test test_name source_file)
        add_executable(${test_name} ${source_file})
        target_include_directories(${test_name} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
        target_compile_definitions(${test_name} PRIVATE
            KUBECHASER_TEST_FIXTURE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/fixtures"
        )
        target_link_libraries(${test_name} PRIVATE kubechaser::app kubechaser::warnings)
        add_test(NAME ${test_name} COMMAND ${test_name})
    endfunction()

    # Run without an OpenGL context: GL calls go to tests/support stubs.
    add_kubechaser_app_test(camera_test camera_test.cpp)
    add_kubechaser_app_test(entity_test entity_test.cpp)
    add_kubechaser_app_test(scene_test scene_test.cpp)

    # Needs a real OpenGL 4.1 context; skipped (exit code 77) without one.
    add_kubechaser_app_test(renderer_test renderer_test.cpp)
    set_tests_properties(renderer_test PROPERTIES SKIP_RETURN_CODE 77 LABELS gpu)
endif()
