set(TEST_SOURCES
    domain/entities/valueobjectstest.cpp
    domain/entities/exercisedefinitiontest.cpp
    domain/repositories/exercisedefinitionquerytest.cpp
    domain/services/exercisematchertest.cpp
)

add_executable(exercisecatalog_tests ${TEST_SOURCES})

target_compile_definitions(exercisecatalog_tests PRIVATE UNIT_TEST)

target_link_libraries(exercisecatalog_tests
    PRIVATE
        exercisecatalog_module
        workout_module
        gtest
        gtest_main
        gmock
        Qt6::Core
)

target_include_directories(exercisecatalog_tests
    PRIVATE
        ${PROJECT_SOURCE_DIR}/src
)

include(GoogleTest)
gtest_discover_tests(exercisecatalog_tests)
