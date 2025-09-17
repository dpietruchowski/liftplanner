find_package(Qt6 6.4 REQUIRED COMPONENTS Quick Sql Qml)

qt_standard_project_setup()

add_subdirectory(lib)
add_subdirectory(models)
add_subdirectory(repositories)
add_subdirectory(services)
add_subdirectory(storage)

if(NOT QML_LIVE_ENABLED)
    add_subdirectory(ui)
endif()

add_subdirectory(ui/icons)
add_subdirectory(utils)

qt_add_executable(liftplanner
    main.cpp
    logging.h
    logging.cpp
    ${PROJECT_SOURCES}
)

qt_add_qml_module(liftplanner
    URI LiftPlanner
    VERSION 1.0
    QML_FILES
        ${PROJECT_QML_SOURCES}
)

qt_add_resources(liftplanner "app_icons"
    PREFIX "/LiftPlanner"
    FILES
        ${PROJECT_ICON_SOURCES}
)

qt_add_resources(liftplanner "resources"
    PREFIX "/LiftPlanner"
    FILES
        data/gpt_prompt_template.txt
)

target_link_libraries(liftplanner
    PRIVATE
    qmllive
    Qt6::Quick
    Qt6::Sql
    Qt6::Qml
)
