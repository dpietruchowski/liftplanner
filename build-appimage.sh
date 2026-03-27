#!/bin/bash
set -e

PROJECT_DIR="$(cd "$(dirname "$0")" && pwd)"
cd "$PROJECT_DIR"

APP_NAME="liftplanner"
VERSION="0.2"
BUILD_DIR="build-release"
APPDIR="AppDir"
OUTPUT_DIR="/home/damian/dev/bin"
SKIP_BUILD=false

# Parse arguments
if [[ "$1" == "--skip-build" ]]; then
    SKIP_BUILD=true
    echo "Skipping build step..."
fi

GREEN='\033[0;32m'
BLUE='\033[0;34m'
RED='\033[0;31m'
NC='\033[0m' # No Color

echo -e "${BLUE}=== Building AppImage for $APP_NAME v$VERSION ===${NC}"

# Create output directory if it doesn't exist
mkdir -p "$OUTPUT_DIR"

LINUXDEPLOY="$OUTPUT_DIR/linuxdeploy-x86_64.AppImage"
LINUXDEPLOY_QT="$OUTPUT_DIR/linuxdeploy-plugin-qt-x86_64.AppImage"

if [ ! -f "$LINUXDEPLOY" ]; then
    echo -e "${GREEN}Downloading linuxdeploy...${NC}"
    wget -q https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage -O "$LINUXDEPLOY"
    chmod +x "$LINUXDEPLOY"
fi

if [ ! -f "$LINUXDEPLOY_QT" ]; then
    echo -e "${GREEN}Downloading linuxdeploy-plugin-qt...${NC}"
    wget -q https://github.com/linuxdeploy/linuxdeploy-plugin-qt/releases/download/continuous/linuxdeploy-plugin-qt-x86_64.AppImage -O "$LINUXDEPLOY_QT"
    chmod +x "$LINUXDEPLOY_QT"
fi

if [ "$SKIP_BUILD" = false ]; then
    echo -e "${GREEN}Cleaning previous build...${NC}"
    rm -rf "$BUILD_DIR" "$APPDIR" *.AppImage

    echo -e "${GREEN}Building application...${NC}"
    cmake -B "$BUILD_DIR" \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_CXX_COMPILER=clang++-18 \
        -DCMAKE_INSTALL_PREFIX=/usr \
        -DQML_LIVE_ENABLED=OFF \
        -DBUILD_TESTING=OFF

    cmake --build "$BUILD_DIR" -j$(nproc)

    echo -e "${GREEN}Installing to AppDir...${NC}"
    DESTDIR="$APPDIR" cmake --install "$BUILD_DIR"
else
    echo -e "${GREEN}Cleaning previous AppDir and AppImage...${NC}"
    rm -rf "$APPDIR" *.AppImage

    echo -e "${GREEN}Installing to AppDir...${NC}"
    DESTDIR="$APPDIR" cmake --install "$BUILD_DIR"
fi

echo -e "${GREEN}Creating desktop file...${NC}"
mkdir -p "$APPDIR/usr/share/applications"
cat > "$APPDIR/usr/share/applications/$APP_NAME.desktop" << EOF
[Desktop Entry]
Type=Application
Name=LiftPlanner
Comment=Workout and lift planning application
Exec=liftplanner
Icon=$APP_NAME
Categories=Health;Sports;Qt;
Terminal=false
EOF

echo -e "${GREEN}Creating icon...${NC}"
mkdir -p "$APPDIR/usr/share/icons/hicolor/256x256/apps"
if [ -f "android/res/mipmap-xxxhdpi/ic_launcher_foreground.png" ]; then
    cp "android/res/mipmap-xxxhdpi/ic_launcher_foreground.png" "$APPDIR/usr/share/icons/hicolor/256x256/apps/$APP_NAME.png"
else
    convert -size 256x256 xc:blue -gravity center -pointsize 100 -fill white -annotate +0+0 "LP" "$APPDIR/usr/share/icons/hicolor/256x256/apps/$APP_NAME.png" 2>/dev/null || echo "Warning: No icon created"
fi

echo -e "${GREEN}Finding Qt installation...${NC}"
export QMAKE=$(which qmake6 || which qmake)
if [ -z "$QMAKE" ]; then
    echo -e "${RED}ERROR: qmake not found${NC}"
    exit 1
fi
echo -e "${GREEN}Using qmake: $QMAKE${NC}"

QT_DIR=$($QMAKE -query QT_INSTALL_PREFIX)
export QML_SOURCES_PATHS="$PROJECT_DIR/src/ui:$PROJECT_DIR/$BUILD_DIR/LiftPlanner"
export LD_LIBRARY_PATH="$QT_DIR/lib:$LD_LIBRARY_PATH"

echo -e "${GREEN}Generating AppImage...${NC}"
export OUTPUT="${APP_NAME}-${VERSION}-x86_64.AppImage"
export LD_LIBRARY_PATH="$PROJECT_DIR/$BUILD_DIR/libs/dbtoolkit:$PROJECT_DIR/$BUILD_DIR/libs/qmllive:$PROJECT_DIR/$BUILD_DIR/libs/qmlcomponents:$LD_LIBRARY_PATH"
"$LINUXDEPLOY" \
    --appdir "$APPDIR" \
    --executable "$APPDIR/usr/bin/liftplanner" \
    --library "$PROJECT_DIR/$BUILD_DIR/libs/dbtoolkit/libdbtoolkit.so" \
    --library "$PROJECT_DIR/$BUILD_DIR/libs/qmllive/libqmllive.so" \
    --library "$PROJECT_DIR/$BUILD_DIR/libs/qmlcomponents/libthemed_components_qml.so" \
    --plugin qt \
    --output appimage

# Move AppImage to output directory
if [ -f "$OUTPUT" ]; then
    mv "$OUTPUT" "$OUTPUT_DIR/"
    echo -e "${BLUE}=== AppImage created successfully: $OUTPUT_DIR/$OUTPUT ===${NC}"
    echo -e "${GREEN}Run with: $OUTPUT_DIR/$OUTPUT${NC}"
else
    echo -e "${RED}ERROR: AppImage generation failed${NC}"
    exit 1
fi
