#!/usr/bin/env bash
set -e

BUILD_DIR=build-android
BUILD_TYPE=Release
CLEAR_BUILD=false
BUILD_TARGET=""
IMAGE_TAG=liftplanner-qt6-android:6.10-api36

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
        aab)
            BUILD_TARGET="--target aab"
            ;;
        apk)
            BUILD_TARGET="--target apk"
            ;;
        *)
            echo "Unknown argument: $arg"
            echo "Usage: ./build.sh [clear] [debug|release] [apk|aab]"
            exit 1
            ;;
    esac
done

if [ "$CLEAR_BUILD" = true ]; then
    rm -rf "$BUILD_DIR"
fi

mkdir -p "$BUILD_DIR"

# Build the SDK-36 image (cached after first run; empty context via stdin).
docker build -t "$IMAGE_TAG" - < Dockerfile.android

docker run --rm \
    -v "${PWD}:/home/user/project:ro" \
    -v "${PWD}/${BUILD_DIR}:/home/user/build" \
    "${IMAGE_TAG}" \
    sh -c "qt-cmake /home/user/project -G Ninja -B /home/user/build -DCMAKE_BUILD_TYPE=${BUILD_TYPE} && cmake --build /home/user/build --config ${BUILD_TYPE} ${BUILD_TARGET}"
