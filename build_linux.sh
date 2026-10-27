#!/usr/bin/env bash
set -euo pipefail
source_dir=$(cd "$(dirname "$0")" && pwd)
command -v qmake >/dev/null || { printf 'Qt 6 qmake is required. See the CI workflow.
' >&2; exit 1; }
mkdir -p "$source_dir/build"
cd "$source_dir/build"
qmake "$source_dir/qFlipper.pro" -spec linux-g++ CONFIG+=release PREFIX="$PWD/AppDir/usr"
make -j"${BUILD_JOBS:-2}"
make install
cp "$source_dir/application/assets/icons/qFlipper.png" AppDir/usr/share/icons/hicolor/512x512/apps/uFlipper.png
cp "$source_dir/installer-assets/appimage/AppRun" AppDir/AppRun
chmod +x AppDir/AppRun
# Test artifact only. Self-contained AppImage packaging follows runtime validation.
tar -czf uFlipper-linux-test.tar.gz AppDir
