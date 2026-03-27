#!/usr/bin/env bash
set -e

echo "📦 Signing APK using apksigner..."

# Extract version from CMakeLists.txt
VERSION=$(grep "set(QT_ANDROID_VERSION_NAME" CMakeLists.txt | sed -n 's/.*"\(.*\)".*/\1/p')
if [ -z "$VERSION" ]; then
    echo "❌ Failed to extract version from CMakeLists.txt"
    exit 1
fi

echo "📌 Version: $VERSION"

BUILD_DIR="build-android"
KEYSTORE="/home/user/project/android/lift-planner.keystore"
KEY_ALIAS="lift_planner_key"
KEYSTORE_PASS="${KEYSTORE_PASS:-liftplanner123}"
UNSIGNED_APK="/home/user/build/src/android-build/build/outputs/apk/release/android-build-release-unsigned.apk"
SIGNED_APK="/home/user/build/liftplanner-${VERSION}-release-signed.apk"

APKSIGNER_PATH="/opt/android-sdk/build-tools/34.0.0/apksigner"

# Sign the APK
echo "🔑 Signing APK..."
docker run --rm \
    -v "${PWD}:/home/user/project:ro" \
    -v "${PWD}/${BUILD_DIR}:/home/user/build" \
    stateoftheartio/qt6:6.8-android-aqt \
    sh -c "cd /home/user/build && \
           ${APKSIGNER_PATH} sign \
               --ks ${KEYSTORE} \
               --ks-key-alias ${KEY_ALIAS} \
               --ks-pass pass:${KEYSTORE_PASS} \
               --out ${SIGNED_APK} \
               ${UNSIGNED_APK}"

echo "✅ APK signed: ${BUILD_DIR}/liftplanner-${VERSION}-release-signed.apk"

# Verify the signature
echo "🔍 Verifying signature..."
if docker run --rm \
    -v "${PWD}:/home/user/project:ro" \
    -v "${PWD}/${BUILD_DIR}:/home/user/build" \
    stateoftheartio/qt6:6.8-android-aqt \
    sh -c "${APKSIGNER_PATH} verify --verbose ${SIGNED_APK}"; then
    echo '✅ Signature verification succeeded!'
else
    echo '❌ Signature verification FAILED!'
    exit 1
fi
