#!/usr/bin/env bash
set -e

BUILD_DIR=build-android
BUILD_TYPE=Release
CLEAR_BUILD=false

for arg in "$@"; do
    case "$arg" in
        clear)
            CLEAR_BUILD=true
            ;;
        debug)
            BUILD_TYPE=Debug
            ;;
        release)
            BUILD_TYPE=Release
            ;;
        *)
            echo "Unknown argument: $arg"
            echo "Usage: ./build.sh [clear] [debug|release]"
            exit 1
            ;;
    esac
done

if [ "$CLEAR_BUILD" = true ]; then
    rm -rf "$BUILD_DIR"
fi

mkdir -p "$BUILD_DIR"

docker run --rm \
    -v "${PWD}:/home/user/project:ro" \
    -v "${PWD}/${BUILD_DIR}:/home/user/build" \
    stateoftheartio/qt6:6.8-android-aqt \
    sh -c "qt-cmake /home/user/project -G Ninja -B /home/user/build -DCMAKE_BUILD_TYPE=${BUILD_TYPE} && cmake --build /home/user/build --config ${BUILD_TYPE}"
