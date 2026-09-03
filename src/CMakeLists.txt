find_package(Qt6 6.4 REQUIRED COMPONENTS Quick Sql Qml QuickControls2)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

qt_standard_project_setup()

add_subdirectory(core)
add_subdirectory(modules/workout)
add_subdirectory(modules/exercisecatalog)
add_subdirectory(modules/userprofile)
add_subdirectory(utils)
add_subdirectory(ui/models)
add_subdirectory(ui/viewmodels)

if(NOT QML_LIVE_ENABLED)
    add_subdirectory(ui/qml)
endif()

qt_add_executable(${APP_TARGET}
    main.cpp
    liftplannerapplication.h
    liftplannerapplication.cpp
)

qt_add_resources(${APP_TARGET} "resources"
    PREFIX "/LiftPlanner"
    FILES
        data/gpt_prompt_template.txt
        data/exercises.json
        data/LGPL-3.txt
        data/GPL-3.txt
)

target_compile_definitions(${APP_TARGET} PRIVATE
    APP_NAME="${APP_NAME}"
    APP_QML_URI="${APP_QML_URI}"
    APP_QML_SOURCE_DIR="${CMAKE_SOURCE_DIR}/${APP_QML_SOURCE_DIR}"
    APP_VERSION="${VERSION_NAME}"
)

target_link_libraries(${APP_TARGET}
    PRIVATE
        core
        workout_module
        exercisecatalog_module
        userprofile_module
        liftplanner_utils
        liftplanner_models
        liftplanner_viewmodels
        app_qmllive
        app_platform
        app_qmlutils
        -Wl,--no-as-needed
        app_theme_qml
        app_themed_qml
        app_components_qml
        app_icons
        -Wl,--as-needed
        Qt6::Quick
        Qt6::QuickControls2
        Qt6::Sql
        Qt6::Qml
        $<$<PLATFORM_ID:Android>:log>
)

if(NOT QML_LIVE_ENABLED)
    target_link_libraries(${APP_TARGET}
        PRIVATE -Wl,--no-as-needed ${APP_QML_TARGET} -Wl,--as-needed)
endif()

if(LIBS_AUTOMATION)
    target_link_libraries(${APP_TARGET} PRIVATE app_automation)
endif()

install(TARGETS ${APP_TARGET}
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)
