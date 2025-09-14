#!/usr/bin/env bash
set -e

BUILD_DIR=build
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
            echo "Nieznany argument: $arg"
            echo "Użycie: ./build.sh [clear] [debug|release]"
            exit 1
            ;;
    esac
done

if [ "$CLEAR_BUILD" = true ]; then
    rm -rf "$BUILD_DIR"
fi

mkdir -p "$BUILD_DIR"

docker run -it --rm \
    -v "${PWD}:/home/user/project:ro" \
    -v "${PWD}/${BUILD_DIR}:/home/user/build" \
    stateoftheartio/qt6:6.8-android-aqt \
    sh -c "qt-cmake ./project -G Ninja -B ./build -DCMAKE_BUILD_TYPE=${BUILD_TYPE}; cmake --build ./build --config ${BUILD_TYPE}"
