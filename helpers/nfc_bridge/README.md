# NFC bridge prototype 0.1.0

Experimental helper for unchanged Momentum `mntm-012`, f7 / API 87.1. Installed after explicit approval on 27 October 2026. Readback hash, start, version query, stop and return to CLI verified on the physical device. Card-session and PC/SC behavior remain unverified. Every helper version needs explicit approval before copying it to the SD card. Artifact details: [build-evidence.json](build-evidence.json).

Build with uFBT using the release SDK specified in the evidence file. `ufbt` builds only; do not use `launch`, `flash`, or `flash_usb` as an implicit installation step. Outputs and editor files are ignored by Git.

App-RPC start argument must be `RPC`. The firmware supplies the context pointer. DataExchange uses `bridge_protocol.h`: eight-byte header, UF magic, version 1, opcode, sequence, status, little-endian payload length. Commands: Info, Activate, Transceive, Deactivate. Transport delivery is acknowledged separately from the correlated protocol result. No RF field starts until Activate.

The NFC worker stays inside the same activated ISO14443-4A session across dependent commands. Disconnect, Back, AppExit, transaction error and timeout stop the poller. AppExit acknowledgement follows NFC shutdown. No card data is saved on the Flipper.

Current prototype limits: ISO14443-4A only; one transaction at a time; APDU length limited to the negotiated card frame minus four bytes, at most 240 bytes. Extended APDU chaining, all-card compatibility and PC/SC integration are not implemented or claimed. A successful build does not prove persistent card state; that still requires a test card and independent client.
