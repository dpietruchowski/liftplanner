set(TEST_SOURCES
    domain/entities/valueobjectstest.cpp
    domain/entities/exercisedefinitiontest.cpp
    domain/repositories/exercisedefinitionquerytest.cpp
    domain/services/exercisematchertest.cpp
    infrastructure/serializers/exercisedefinitionserializertest.cpp
    infrastructure/database/exercisedefinitionrepositorydbtest.cpp
    infrastructure/seed/exercisecatalogseedtest.cpp
    application/exercisecatalogservicetest.cpp
)

add_executable(exercisecatalog_tests ${TEST_SOURCES})

target_compile_definitions(exercisecatalog_tests PRIVATE
    UNIT_TEST
    EXERCISE_SEED_FILE="${PROJECT_SOURCE_DIR}/src/data/exercises.json"
)

target_link_libraries(exercisecatalog_tests
    PRIVATE
        core
        exercisecatalog_module
        app_utils
        workout_module
        dbtoolkit
        gtest
        gtest_main
        gmock
        Qt6::Core
        Qt6::Sql
)

target_include_directories(exercisecatalog_tests
    PRIVATE
        ${PROJECT_SOURCE_DIR}/src
)

include(GoogleTest)
gtest_discover_tests(exercisecatalog_tests)
