#!/usr/bin/env bash
set -e

echo "📦 Signing APK using apksigner..."

BUILD_DIR="build"
KEYSTORE="/home/user/project/android/lift-planner.keystore"
KEY_ALIAS="lift_planner_key"
UNSIGNED_APK="/home/user/build/src/android-build/build/outputs/apk/release/android-build-release-unsigned.apk"
SIGNED_APK="/home/user/build/liftplanner-release-signed.apk"

APKSIGNER_PATH="/opt/android-sdk/build-tools/34.0.0/apksigner"

# Sign the APK
docker run -it --rm \
    -v "${PWD}:/home/user/project:ro" \
    -v "${PWD}/${BUILD_DIR}:/home/user/build" \
    stateoftheartio/qt6:6.8-android-aqt \
    sh -c "cd /home/user/build && \
           echo '🔑 Signing APK...' && \
           $APKSIGNER_PATH sign \
               --ks '$KEYSTORE' \
               --ks-key-alias '$KEY_ALIAS' \
               --out '$SIGNED_APK' \
               '$UNSIGNED_APK'"

echo "✅ APK signed: ${SIGNED_APK}"

# Verify the signature
docker run -it --rm \
    -v "${PWD}:/home/user/project:ro" \
    -v "${PWD}/${BUILD_DIR}:/home/user/build" \
    stateoftheartio/qt6:6.8-android-aqt \
    sh -c "$APKSIGNER_PATH verify --verbose '$SIGNED_APK'"

if [ $? -eq 0 ]; then
    echo '✅ Signature verification succeeded!'
else
    echo '❌ Signature verification FAILED!'
    exit 1
fi
