cmake_minimum_required(VERSION 3.16)

project(liftplanner VERSION 1.0 LANGUAGES CXX)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Qml Quick QuickControls2)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Android
set(ANDROID_PACKAGE_NAME "com.liftplanner.app")
set(ANDROID_KEYSTORE_PATH "${CMAKE_SOURCE_DIR}/android/lift-planner.keystore")
set(ANDROID_KEY_ALIAS "lift_planner_key")
set(QT_ANDROID_VERSION_CODE 6)
set(QT_ANDROID_VERSION_NAME "1.0-rc5")
set(QT_ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_SOURCE_DIR}/android")

set(QML_LIVE_ENABLED OFF)

if(NOT ANDROID)
    add_compile_definitions(LIBS_TESTING)
endif()

find_program(CLANG_FORMAT "clang-format")

if(CLANG_FORMAT)
    file(GLOB_RECURSE FORMAT_FILES CONFIGURE_DEPENDS
        ${CMAKE_SOURCE_DIR}/src/*.cpp ${CMAKE_SOURCE_DIR}/src/*.h ${CMAKE_SOURCE_DIR}/src/*.hpp
        ${CMAKE_SOURCE_DIR}/tests/*.cpp ${CMAKE_SOURCE_DIR}/tests/*.h ${CMAKE_SOURCE_DIR}/tests/*.hpp)
    list(FILTER FORMAT_FILES EXCLUDE REGEX "/tests/third_party/")

    add_custom_target(format
        COMMAND ${CLANG_FORMAT} -style=file -i ${FORMAT_FILES}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Formatting src/ and tests/"
    )

    add_custom_target(format_check
        COMMAND ${CLANG_FORMAT} -style=file --dry-run -Werror ${FORMAT_FILES}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Checking format of src/ and tests/"
    )

    message(STATUS "Clang-format found: ${CLANG_FORMAT}")
else()
    message(WARNING "Clang-format not found")
endif()

add_subdirectory(libs)
add_subdirectory(src)

if(NOT ANDROID)
    enable_testing()
    add_subdirectory(tests)
endif()

if(ANDROID)
    set(CMAKE_BUILD_TYPE Release)
    set(QML_LIVE_ENABLED OFF)
    set_target_properties(liftplanner PROPERTIES
        QT_ANDROID_PACKAGE_NAME   "${ANDROID_PACKAGE_NAME}"
        QT_ANDROID_VERSION_CODE   "${QT_ANDROID_VERSION_CODE}"
        QT_ANDROID_VERSION_NAME   "${QT_ANDROID_VERSION_NAME}"
        QT_ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_SOURCE_DIR}/android"
        QT_ANDROID_KEYSTORE_PATH "${ANDROID_KEYSTORE_PATH}"
        QT_ANDROID_KEY_ALIAS "${ANDROID_KEY_ALIAS}"
    )

    qt_android_generate_deployment_settings(liftplanner)
    qt_android_add_apk_target(liftplanner)

    get_target_property(QT_ANDROID_DIR liftplanner QT_ANDROID_PACKAGE_SOURCE_DIR)
    message(STATUS "Target property QT_ANDROID_PACKAGE_SOURCE_DIR = ${QT_ANDROID_DIR}")
endif()

if(QML_LIVE_ENABLED)
    add_definitions(-DQML_LIVE_ENABLED)
endif()