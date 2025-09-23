#!/usr/bin/env bash
set -e

echo "📦 Podpisywanie APK przy użyciu jarsigner..."

BUILD_DIR="build"
KEYSTORE="/home/user/project/android/lift-planner.keystore"
KEY_ALIAS="lift_planner_key"
UNSIGNED_APK="/home/user/build/outputs/apk/release/android-build-release-unsigned.apk"
SIGNED_APK="/home/user/build/liftplanner-release-signed.apk"

docker run -it --rm \
    -v "${PWD}:/home/user/project:ro" \
    -v "${PWD}/${BUILD_DIR}:/home/user/build" \
    stateoftheartio/qt6:6.8-android-aqt \
    sh -c "cd /home/user/build && \
           jarsigner -verbose -sigalg SHA256withRSA -digestalg SHA-256 \
           -keystore '$KEYSTORE' \
           'src/android-build/build/outputs/apk/release/android-build-release-unsigned.apk' \
           '$KEY_ALIAS' && \
           cp 'src/android-build/build/outputs/apk/release/android-build-release-unsigned.apk' 'liftplanner-release-signed.apk'"

echo "✅ APK podpisane: ${BUILD_DIR}/liftplanner-release-signed.apk"

# Sprawdzenie podpisu
docker run -it --rm \
    -v "${PWD}:/home/user/project:ro" \
    -v "${PWD}/${BUILD_DIR}:/home/user/build" \
    stateoftheartio/qt6:6.8-android-aqt \
    sh -c "jarsigner -verify -verbose -certs '/home/user/build/liftplanner-release-signed.apk'"

if [ $? -eq 0 ]; then
  echo '✅ Weryfikacja podpisu powiodła się!'
else
  echo '❌ Weryfikacja podpisu NIE powiodła się!'
  exit 1
fi