# uFlipper

A desktop workbench for Flipper Zero, built on qFlipper. macOS and Linux are the target platforms; Momentum remains unmodified.

**Development in progress — no public release yet.** Device integration and the full feature set are being implemented and verified. A successful core test does not certify the complete application.

## The workbench

- Device dashboard with firmware, storage, battery and power information.
- Desktop tools for supported IR, Sub-GHz, NFC, LF RFID, iButton, GPIO/buses, USB/Bluetooth functions and device apps.
- Automatically saved live sessions and a searchable local library.
- Firmware sources, verified device backups and restore workflows.
- Google Drive, Dropbox, OneDrive, WebDAV and S3 synchronization, optional encryption and a shared trash.
- Local MCP integration and a planned macOS NFC system reader.

These are the accepted product goals, **not a claim that every item is already available**. Current implementation and observed results: [foundation task](docs/tasks/T-0003_foundation/00_brief.md). Device capabilities and limits: [function matrix](docs/tasks/T-0002_technical-plan/06_function_matrix.md).

## Development

The application uses C++17, Qt 6 and qmake. Portable core checks use a small separate CMake target:

```sh
cmake -S workbench -B build-core -DCMAKE_BUILD_TYPE=Debug
cmake --build build-core --parallel 2
ctest --test-dir build-core --output-on-failure
```

Core dependencies: SQLite3, libsodium and pkg-config. Desktop builds additionally require Qt Quick/Quick Controls, SerialPort, SVG, Core5Compat, zlib and libusb. Initialize the nanopb submodule before a full build. See the fork-owned CI workflow for the pinned Qt toolchain and test artifacts.

Release targets are macOS 13+ (Apple Silicon and Intel Universal) and Ubuntu 22.04+ x86_64 (AppImage). Minimum-version runtime certification and distribution packaging remain release checks. No simulator is required.

## Credits and license

uFlipper is a fork of [qFlipper](https://github.com/flipperdevices/qFlipper) by Flipper Devices and its contributors, distributed under the existing [GNU GPLv3](LICENSE). Existing third-party licenses and notices remain applicable. [Momentum](https://github.com/Next-Flip/Momentum-Firmware) is an independent firmware project; uFlipper does not imply endorsement by either upstream project.
