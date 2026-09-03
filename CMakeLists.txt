cmake_minimum_required(VERSION 3.16)

project(liftplanner VERSION 1.0 LANGUAGES CXX)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Qml Quick QuickControls2 Sql)

include(libs/cmake/AppProject.cmake)

app_load_env(${CMAKE_SOURCE_DIR}/app.env)

option(BUILD_TESTING "Build the test suite" ON)

app_project_setup(
    SOURCE_ROOT ${CMAKE_SOURCE_DIR}/src
    FORMAT_DIRS src tests
)

set(LIBS_BUILD_TESTS OFF CACHE BOOL "" FORCE)

add_subdirectory(libs)
add_subdirectory(src)

if(NOT ANDROID AND BUILD_TESTING)
    add_subdirectory(tests)
endif()

if(ANDROID)
    app_configure_android(${APP_TARGET}
        PACKAGE_NAME "${ANDROID_PACKAGE}"
        VERSION_CODE ${VERSION_CODE}
        VERSION_NAME "${VERSION_NAME}"
        KEYSTORE "${CMAKE_SOURCE_DIR}/${ANDROID_KEYSTORE}"
        KEY_ALIAS "${ANDROID_KEY_ALIAS}"
        PACKAGE_SOURCE_DIR ${CMAKE_SOURCE_DIR}/android
    )
endif()
