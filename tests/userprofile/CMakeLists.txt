app_add_test(userprofile_tests
    DEPS
        domain
        infrastructure
        application
        app_dbtoolkit
        Qt6::Sql
)
