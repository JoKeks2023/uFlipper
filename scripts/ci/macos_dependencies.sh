#!/usr/bin/env bash
set -euo pipefail
prefix=${1:?Provide a temporary dependency directory}
mkdir -p "$prefix/sources" "$prefix/universal/lib/pkgconfig" "$prefix/universal/include"
download() {
    local url=$1 file=$2 digest=$3
    curl --fail --location --retry 3 "$url" -o "$file"
    printf '%s  %s
' "$digest" "$file" | shasum -a 256 -c -
}
download https://github.com/libusb/libusb/releases/download/v1.0.30/libusb-1.0.30.tar.bz2 "$prefix/libusb.tar.bz2" fea36f34f9156400209595e300840767ab1a385ede1dc7ee893015aea9c6dbaf
download https://github.com/jedisct1/libsodium/releases/download/1.0.22-RELEASE/libsodium-1.0.22.tar.gz "$prefix/libsodium.tar.gz" adbdd8f16149e81ac6078a03aca6fc03b592b89ef7b5ed83841c086191be3349
tar -xf "$prefix/libusb.tar.bz2" -C "$prefix/sources"
tar -xf "$prefix/libsodium.tar.gz" -C "$prefix/sources"
for arch in arm64 x86_64; do
    for package in libusb-1.0.30 libsodium-1.0.22; do
        mkdir -p "$prefix/build-$package-$arch"
        (
            cd "$prefix/build-$package-$arch"
            CC=clang CFLAGS="-arch $arch -mmacosx-version-min=13.0" LDFLAGS="-arch $arch -mmacosx-version-min=13.0" \
                "$prefix/sources/$package/configure" --host="$arch-apple-darwin" --prefix="$prefix/$arch" --disable-shared --enable-static
            make -j3
            make install
        )
    done
done
cp -R "$prefix/arm64/include/." "$prefix/universal/include/"
for library in libusb-1.0 libsodium; do
    lipo -create "$prefix/arm64/lib/$library.a" "$prefix/x86_64/lib/$library.a" -output "$prefix/universal/lib/$library.a"
    lipo -archs "$prefix/universal/lib/$library.a"
done
for package in libusb-1.0 libsodium; do
    sed "s|^prefix=.*|prefix=$prefix/universal|" "$prefix/arm64/lib/pkgconfig/$package.pc" > "$prefix/universal/lib/pkgconfig/$package.pc"
done
