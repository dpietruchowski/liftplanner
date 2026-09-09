app_add_module(ui
    EXCLUDE_REGEX "/qml/"
    DEPS
        application
        app_platform
        Qt6::Core
        Qt6::Gui
        Qt6::Qml
        Qt6::Quick
)
