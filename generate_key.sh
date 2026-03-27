#!/usr/bin/env bash
set -e

KEYSTORE_DIR="android"
KEYSTORE_FILE="${KEYSTORE_DIR}/lift-planner.keystore"
KEY_ALIAS="lift_planner_key"
KEYSTORE_PASS="${KEYSTORE_PASS:-liftplanner123}"
KEY_VALIDITY_DAYS=10000

echo "🔐 Generating new keystore for Android signing..."

if [ -f "$KEYSTORE_FILE" ]; then
    echo "⚠️  Keystore already exists at: $KEYSTORE_FILE"
    read -p "Do you want to overwrite it? (yes/no): " CONFIRM
    if [ "$CONFIRM" != "yes" ]; then
        echo "❌ Aborted."
        exit 1
    fi
    rm -f "$KEYSTORE_FILE"
fi

mkdir -p "$KEYSTORE_DIR"

echo "📝 Generating keystore..."
echo ""

docker run --rm -i \
    -v "${PWD}/${KEYSTORE_DIR}:/home/user/keystore" \
    stateoftheartio/qt6:6.8-android-aqt \
    sh -c "keytool -genkeypair \
        -v \
        -keystore /home/user/keystore/lift-planner.keystore \
        -alias ${KEY_ALIAS} \
        -keyalg RSA \
        -keysize 2048 \
        -validity ${KEY_VALIDITY_DAYS} \
        -storepass ${KEYSTORE_PASS} \
        -keypass ${KEYSTORE_PASS} \
        -dname 'CN=LiftPlanner App, OU=Development, O=LiftPlanner, L=Warsaw, ST=Masovian, C=PL'"

echo ""
echo "✅ Keystore generated successfully!"
echo "📁 Location: $KEYSTORE_FILE"
echo "🔑 Alias: $KEY_ALIAS"
echo "🔒 Password: $KEYSTORE_PASS"
echo ""
echo "⚠️  Keep this keystore file secure and never commit it to version control!"
