# Experimental macOS reader component

Build only: `python3 scripts/reader/build_macos_reader.py NEW_OUTPUT_DIRECTORY`. The script pins and verifies virtualsmartcard and PCSC header sources, builds a macOS 13+ Universal bundle and writes a final binary hash. It never installs a system component. GPLv3 upstream source/license and header notices remain with the review artifact.

The native app supplies the VICC endpoint in reverse mode at **127.0.0.1:35963**, only while the user explicitly enables Mac-Systemreader. One slot, short APDUs, maximum 240 command/256 response bytes; the negotiated card limit can be lower. A transport ATR `3B800181` describes the T=1 proxy and is not read from the contactless card. Card APDU responses pass through unchanged.

`test_driver_contract.py OUTPUT_DIRECTORY` loads the bundle directly without registering it, using a synthetic local peer. It proves driver ABI/framing/power/transmit limits, not an OS reader slot or independent-client functionality.

Before installation, present the exact artifact/hash for explicit approval. Intended registration location: `/usr/local/libexec/SmartCardServices/drivers/uFlipperReader.bundle`, USB trigger 0483:5740 (Flipper serial mode). Preserve any prior bundle; removal of this one bundle reverses the installation. Do not replace Apple drivers or disable system protections. Current macOS compatibility must be observed; the upstream historical guide does not prove it.

Still required: actual reader enumeration, OpenSC access, dependent card commands and removal/reset/timeout/reconnect checks. Helper 0.1.0 does not proactively report a removed idle card; no claim of complete reader lifecycle. Any necessary new helper version needs its own explicit installation approval.

Sources: [virtualsmartcard macOS integration](https://github.com/frankmorgner/vsmartcard/blob/93286a90d2411120ae17299f9902e1fdd8954185/virtualsmartcard/doc/README.txt.in), [PCSC IFD API](https://github.com/LudovicRousseau/PCSC/blob/8926ecb11ece8f1d2faf9296117869d366ecc6dc/src/PCSC/ifdhandler.h).
