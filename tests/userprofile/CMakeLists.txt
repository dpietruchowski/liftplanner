set(TEST_SOURCES
    domain/entities/userprofiletest.cpp
    infrastructure/serializers/userprofileserializertest.cpp
    infrastructure/database/userprofilerepositorydbtest.cpp
    application/userprofileservicetest.cpp
)

add_executable(userprofile_tests ${TEST_SOURCES})

target_compile_definitions(userprofile_tests PRIVATE UNIT_TEST)

target_link_libraries(userprofile_tests
    PRIVATE
        userprofile_module
        dbtoolkit
        gtest
        gtest_main
        gmock
        Qt6::Core
        Qt6::Sql
)

target_include_directories(userprofile_tests
    PRIVATE
        ${PROJECT_SOURCE_DIR}/src
)

include(GoogleTest)
gtest_discover_tests(userprofile_tests)
