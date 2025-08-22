file(GLOB QML_FILES "*.qml")
set(PROJECT_QML_SOURCES "")
foreach(qml_file ${QML_FILES})
    get_filename_component(file_name ${qml_file} NAME)
    list(APPEND PROJECT_QML_SOURCES "ui/${file_name}")
endforeach()

set(PROJECT_QML_SOURCES ${PROJECT_QML_SOURCES} PARENT_SCOPE)