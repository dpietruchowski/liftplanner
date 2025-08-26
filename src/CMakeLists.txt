find_package(Qt6 6.4 REQUIRED COMPONENTS Quick Sql Qml)

qt_standard_project_setup()

add_subdirectory(models)
add_subdirectory(repositories)
add_subdirectory(services)
add_subdirectory(storage)
add_subdirectory(ui)
add_subdirectory(utils)

qt_add_executable(liftplanner
    main.cpp
    logging.h
    logging.cpp
    ${PROJECT_SOURCES}
)
set_source_files_properties(ui/Theme.qml
    PROPERTIES QT_QML_SINGLETON_TYPE TRUE)
qt_add_qml_module(liftplanner
    URI LiftPlanner
    VERSION 1.0
    QML_FILES 
        ${PROJECT_QML_SOURCES}
)

target_link_libraries(liftplanner
    PRIVATE
    Qt6::Quick
    Qt6::Sql
    Qt6::Qml
)
