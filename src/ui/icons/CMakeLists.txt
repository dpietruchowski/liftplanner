file(GLOB ICON_FILES "*.svg")

set(PROJECT_ICON_SOURCES "")

foreach(icon_file ${ICON_FILES})
    get_filename_component(file_name ${icon_file} NAME)
    list(APPEND PROJECT_ICON_SOURCES "ui/icons/${file_name}")
endforeach()

set(PROJECT_ICON_SOURCES ${PROJECT_ICON_SOURCES} PARENT_SCOPE)