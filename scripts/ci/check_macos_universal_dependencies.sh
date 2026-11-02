#!/usr/bin/env bash
set -euo pipefail
uflipper_expected_prefix=${1:?Provide the universal dependency prefix}
for uflipper_module in libusb-1.0 libsodium; do
    uflipper_actual_prefix=$(pkg-config --variable=prefix "$uflipper_module")
    if [[ "$uflipper_actual_prefix" != "$uflipper_expected_prefix" ]]; then
        printf 'Wrong %s prefix: %s (expected %s)\n' "$uflipper_module" "$uflipper_actual_prefix" "$uflipper_expected_prefix" >&2
        exit 1
    fi
done
for uflipper_archive in libusb-1.0.a libsodium.a; do
    lipo "$uflipper_expected_prefix/lib/$uflipper_archive" -verify_arch x86_64 arm64
done
