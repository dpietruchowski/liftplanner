find_package(Qt6 REQUIRED COMPONENTS Test)

set(CMAKE_AUTOMOC ON)

set(TEST_SOURCES
    main.cpp
    testapplication.cpp
    workout_lifecycle_test.cpp
    planned_workout_test.cpp
    active_workout_test.cpp
)

set(APP_SOURCES
    ${PROJECT_SOURCE_DIR}/src/ui/viewmodels/activeworkoutviewmodel.cpp
    ${PROJECT_SOURCE_DIR}/src/ui/viewmodels/plannedworkoutviewmodel.cpp
    ${PROJECT_SOURCE_DIR}/src/ui/viewmodels/workouthistoryviewmodel.cpp
    ${PROJECT_SOURCE_DIR}/src/ui/models/workoutmodel.cpp
    ${PROJECT_SOURCE_DIR}/src/ui/models/exercisemodel.cpp
    ${PROJECT_SOURCE_DIR}/src/ui/models/setmodel.cpp
    ${PROJECT_SOURCE_DIR}/src/utils/workoutjson.cpp
    ${PROJECT_SOURCE_DIR}/src/utils/workouttext.cpp
)

add_executable(integration_tests ${TEST_SOURCES} ${APP_SOURCES})

target_compile_definitions(integration_tests PRIVATE UNIT_TEST)

target_link_libraries(integration_tests
    PRIVATE
        core
        workout_module
        userprofile_module
        dbtoolkit
        gtest
        gmock
        Qt6::Core
        Qt6::Sql
        Qt6::Test
)

target_include_directories(integration_tests
    PRIVATE
        ${PROJECT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}
)

include(GoogleTest)
gtest_discover_tests(integration_tests)
