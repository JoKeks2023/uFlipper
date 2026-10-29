# uFlipper shared core

Portable C++17 building blocks for the Qt application and local integrations. The application remains C++/Qt/QML; CMake is used only for small isolated core tests.

- `OperationCoordinator`: one generation of device ownership, explicit conflict decisions, stop acknowledgement and uncertain outcomes. An unconfirmed failure retains hardware ownership.
- `CaptureStore`: SQLite catalog, exclusive process lease, bounded binary records, SHA256 verification, recoverable interrupted recordings. Recordings are not queued in RAM.
- `BackupStore`: versioned manifests and streamed payloads, two required inventories for a full-device backup, explicit test-subset scope and corruption/crash checks. The native `BackupController` now traverses both storage roots through the existing RPC owner; `BackupSession` streams files, checks remote MD5 immediately and after a second inventory, then verifies local payloads on a worker. Full-device runtime verification is tracked under T-0010. Product restore and the fresh automatic maintenance preflight remain unconnected.
- `FileCrypto`: libsodium authenticated streams, Argon2id derivation and versioned recovery-key encoding. This module is not yet a complete cloud key-management implementation.

## Isolated checks

```sh
cmake -S workbench -B build-core -DCMAKE_BUILD_TYPE=Debug
cmake --build build-core --parallel 2
ctest --test-dir build-core --output-on-failure
```

Dependencies: SQLite3 and libsodium, discoverable through the system SDK/pkg-config. No simulator or full Qt build is needed.

## Recording format 1

A session directory contains `manifest.json` and `events.bin`. The catalog is `catalog.sqlite` at the storage root. Session IDs are generated 128-bit random hex identifiers. The manifest preserves kind, caller-supplied metadata, state, reason, byte length, checksum and timebase.

`events.bin`: 8-byte magic `UFCAP001`, then records of one type byte (`0` data, `1` gap, `2` status), an unsigned little-endian 64-bit host monotonic timestamp in nanoseconds, a little-endian 32-bit payload length, and that many original bytes. A record is limited to 1 MiB. Device timing belongs in the original payload/metadata; host receipt time does not claim a device-clock timestamp.

Interrupted sessions retain original bytes; integrity/record validation distinguishes incomplete tails. Completed files are verified before being treated as intact.

## Encrypted-file format 1

8-byte magic `UFENC\0\0\1`, the libsodium secretstream header, then big-endian 32-bit length-prefixed ciphertext chunks. Plaintext chunks are at most 64 KiB. A final authenticated tag is mandatory, and trailing ciphertext is rejected. Decryption publishes the output only after complete authentication and never overwrites an existing output. Algorithm is `crypto_secretstream_xchacha20poly1305`.

Password derivation currently uses Argon2id13 with libsodium's interactive limits. Salt and actual parameters must be persisted by the key-envelope layer before cloud use. Recovery codes have the `ufkey1:` prefix and URL-safe base64 encoding. Cloud adapters and key-envelope migration are separate implementation steps.

## Physical-device development probes

With the pinned `3rdparty/nanopb` submodule initialized, the small CMake build also builds two bounded RPC probes. They use their own exclusive serial connection; close the desktop app before running either probe. These tools are development evidence, not replacement product interfaces.

- `uflipper_helper_lifecycle_probe --port PATH --store DIRECTORY`: starts the already approved and installed NFC helper 0.1.0, checks its version and confirms stop/return to CLI. It does not install a helper or activate a card.
- `uflipper_backup_fixture_probe --port PATH --store DIRECTORY`: creates a fresh random directory below `/ext/uflipper-tests/`, writes two own fixtures plus an empty directory, reads and verifies an original tests-only backup, changes the files, creates a second verified backup before restoring, restores from original payloads and compares all bytes read back from the device. It removes only this invocation's test directory after success; failures retain it for diagnosis. No caller-supplied device path is accepted. This cannot satisfy the full-backup firmware preflight.

Both probes persist bounded raw RPC traces in the recording catalog. Backup payloads and manifests stay in the separate backup catalog. The Qt `uFlipper-probe` is built only with the desktop application; its optional card test requires an appropriate ISO-DEP Type 4A NDEF test card and is not an independent PC/SC-client proof.

## Independent backup verification

After the native app has finished a backup, run `python3 scripts/backup/verify_backup.py BACKUP_DIRECTORY --expect-full` from the repository root. This read-only development check independently compares catalog and manifest, hashes each payload with bounded buffers and checks both root confirmations. It never opens the device or changes backup state. A test-subset backup is explicitly rejected by `--expect-full`.

Remote comparisons detect changes across the two verification passes; the device interface does not offer an atomic filesystem snapshot. Changes during transfer cause an interrupted backup rather than a verified result.
