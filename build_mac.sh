#!/usr/bin/env bash
set -euo pipefail
source_dir=$(cd "$(dirname "$0")" && pwd)
command -v qmake >/dev/null || { printf 'Qt 6 qmake is required. See the CI workflow.
' >&2; exit 1; }
mkdir -p "$source_dir/build_mac"
cd "$source_dir/build_mac"
qmake "$source_dir/qFlipper.pro" -spec macx-clang CONFIG+=release QMAKE_APPLE_DEVICE_ARCHS="x86_64 arm64" QMAKE_MACOSX_DEPLOYMENT_TARGET=13.0
make -j"${BUILD_JOBS:-2}"
make install
macdeployqt uFlipper.app -executable=uFlipper.app/Contents/MacOS/uFlipper-cli
lipo -archs uFlipper.app/Contents/MacOS/uFlipper
/usr/libexec/PlistBuddy -c 'Print CFBundleIdentifier' uFlipper.app/Contents/Info.plist
/usr/bin/ditto -c -k --keepParent uFlipper.app uFlipper.app.zip
mkdir -p dmg-stage
/usr/bin/ditto uFlipper.app dmg-stage/uFlipper.app
ln -sfn /Applications dmg-stage/Applications
hdiutil create -volname uFlipper -srcfolder dmg-stage -format UDZO -ov uFlipper.dmg
