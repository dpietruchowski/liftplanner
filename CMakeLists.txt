cmake_minimum_required(VERSION 3.16)

project(liftplanner VERSION 0.2 LANGUAGES CXX)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Qml Quick)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED OFF)

# Android
set(ANDROID_PACKAGE_NAME "com.liftplanner.app")
set(QT_ANDROID_VERSION_CODE 1)
set(QT_ANDROID_VERSION_NAME "0.1")
set(QT_ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_SOURCE_DIR}/android")

set(QML_LIVE_ENABLED OFF)
if(QML_LIVE_ENABLED)
    add_definitions(-DQML_LIVE_ENABLED)
endif()

add_subdirectory(src)

if(ANDROID)
    set_target_properties(liftplanner PROPERTIES
        QT_ANDROID_PACKAGE_NAME   "${ANDROID_PACKAGE_NAME}"
        QT_ANDROID_VERSION_CODE   "${QT_ANDROID_VERSION_CODE}"
        QT_ANDROID_VERSION_NAME   "${QT_ANDROID_VERSION_NAME}"
        QT_ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_SOURCE_DIR}/android"
    )

    qt_android_generate_deployment_settings(liftplanner)
    qt_android_add_apk_target(liftplanner)

    get_target_property(QT_ANDROID_DIR liftplanner QT_ANDROID_PACKAGE_SOURCE_DIR)
    message(STATUS "Target property QT_ANDROID_PACKAGE_SOURCE_DIR = ${QT_ANDROID_DIR}")
endif()
