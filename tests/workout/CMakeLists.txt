app_add_test(workout_tests
    DEPS
        core
        workout_module
        liftplanner_utils
        app_dbtoolkit
        Qt6::Sql
)
