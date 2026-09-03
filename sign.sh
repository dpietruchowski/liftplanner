#!/usr/bin/env bash
set -e

ARTIFACT="${1:-aab}"
case "$ARTIFACT" in
    apk|aab) ;;
    *)
        echo "Usage: KEYSTORE_PASS=... ./sign.sh [apk|aab]"
        exit 1
        ;;
esac

if [ -z "$KEYSTORE_PASS" ]; then
    echo "❌ KEYSTORE_PASS is not set. Export the keystore password before signing."
    exit 1
fi

VERSION=$(grep "set(QT_ANDROID_VERSION_NAME" CMakeLists.txt | sed -n 's/.*"\(.*\)".*/\1/p')
if [ -z "$VERSION" ]; then
    echo "❌ Failed to extract version from CMakeLists.txt"
    exit 1
fi

echo "📌 Version: $VERSION"

BUILD_DIR="build-android"
IMAGE_TAG=liftplanner-qt6-android:6.10-api36
KEYSTORE="/home/user/project/android/lift-planner.keystore"
KEY_ALIAS="lift_planner_key"
OUTPUTS="/home/user/build/src/android-build/build/outputs"
APKSIGNER_PATH="/opt/android-sdk/build-tools/36.0.0/apksigner"

run_in_image() {
    docker run --rm \
        -e KEYSTORE_PASS \
        -v "${PWD}:/home/user/project:ro" \
        -v "${PWD}/${BUILD_DIR}:/home/user/build" \
        "${IMAGE_TAG}" \
        sh -c "$1"
}

if [ "$ARTIFACT" = "apk" ]; then
    UNSIGNED="${OUTPUTS}/apk/release/android-build-release-unsigned.apk"
    SIGNED="/home/user/build/liftplanner-${VERSION}-release-signed.apk"

    echo "🔑 Signing APK with apksigner..."
    run_in_image "${APKSIGNER_PATH} sign \
        --ks ${KEYSTORE} \
        --ks-key-alias ${KEY_ALIAS} \
        --ks-pass env:KEYSTORE_PASS \
        --out ${SIGNED} \
        ${UNSIGNED}"

    echo "🔍 Verifying signature..."
    run_in_image "${APKSIGNER_PATH} verify --verbose ${SIGNED}"
else
    UNSIGNED="${OUTPUTS}/bundle/release/android-build-release.aab"
    SIGNED="/home/user/build/liftplanner-${VERSION}-release-signed.aab"

    echo "🔑 Signing AAB with jarsigner..."
    run_in_image "jarsigner \
        -keystore ${KEYSTORE} \
        -storepass:env KEYSTORE_PASS \
        -sigalg SHA256withRSA \
        -digestalg SHA-256 \
        -signedjar ${SIGNED} \
        ${UNSIGNED} ${KEY_ALIAS}"

    echo "🔍 Verifying signature..."
    run_in_image "jarsigner -verify -verbose:summary ${SIGNED}"
fi

echo "✅ Signed: ${BUILD_DIR}/$(basename "${SIGNED}")"
