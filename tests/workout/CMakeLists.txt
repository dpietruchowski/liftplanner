set(TEST_SOURCES
    domain/entities/workouttest.cpp
    domain/entities/exercisetest.cpp
    domain/entities/settest.cpp
    infrastructure/serializers/workoutserializertest.cpp
    infrastructure/serializers/exerciseserializertest.cpp
    infrastructure/serializers/setserializertest.cpp
    infrastructure/database/workoutrepositorydbtest.cpp
)

add_executable(workout_tests ${TEST_SOURCES})

target_compile_definitions(workout_tests PRIVATE UNIT_TEST)

target_link_libraries(workout_tests
    PRIVATE
        core
        workout_module
        dbtoolkit
        gtest
        gtest_main
        gmock
        Qt6::Core
        Qt6::Sql
)

target_include_directories(workout_tests
    PRIVATE
        ${PROJECT_SOURCE_DIR}/src
)

include(GoogleTest)
gtest_discover_tests(workout_tests)
