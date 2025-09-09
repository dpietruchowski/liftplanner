cmake_minimum_required(VERSION 3.16)

project(liftplanner VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(ANDROID_PACKAGE_NAME "com.liftplanner.app")

set(QML_LIVE_ENABLED OFF)
if(QML_LIVE_ENABLED)
    add_definitions(-DQML_LIVE_ENABLED)
endif()

add_subdirectory(src)
