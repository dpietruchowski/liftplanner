app_add_test(infrastructure_tests
    DEPS
        infrastructure
        app_dbtoolkit
        Qt6::Sql
    DEFINITIONS
        EXERCISE_SEED_FILE="${PROJECT_SOURCE_DIR}/src/data/exercises.json"
)
