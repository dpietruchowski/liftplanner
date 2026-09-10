app_add_test(ui_tests
    OWN_MAIN
    DEPS
        ui
        Qt6::Gui
    DEFINITIONS
        QML_SOURCE_DIR="${CMAKE_SOURCE_DIR}/src/ui/qml"
)
