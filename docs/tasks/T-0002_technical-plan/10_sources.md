# Quellen und Beleggrenzen

Stand: 27.10.2026. Dies ist eine Quellprüfung, kein Geräte- oder Betriebssystemtest.

- Fork: `JoKeks2023/uFlipper`, Branch `dev`, Commit `1d26683ff5b751f219a4e40443b7c8355eb39469`.
- Firmware: Momentum `mntm-012`, Commit `e1784e7418d8b074e971983ceb6fef0f37e52ae4`. Belegt ist dieser Stand, keine beliebige zukünftige Firmware.
- 51 Firmwaredateien und 17 Dateien des Forks sind mit Pfad, Revision, Bytezahl und SHA256 in [sources.json](sources.json) verzeichnet. Die Hashes identifizieren die gelesenen Dateien; sie sind keine Herstellersignatur.
- Öffentliche Dokumentation und Update-Metadaten wurden am 27.10.2026 lesend geprüft. Diese URLs können sich ändern. Keine Firmwarepakete heruntergeladen oder installiert.
- Der frühere eigenständige Prototyp und frühere Gerätetests gelten nicht als Laufzeitnachweis dieses Forks.

## Fork-Belege

| ID | Quelle | Relevanz |
| --- | --- | --- |
| Q01 | [backend/flipperzero/protobufsession.h](https://github.com/JoKeks2023/uFlipper/blob/1d26683ff5b751f219a4e40443b7c8355eb39469/backend/flipperzero/protobufsession.h) | Host-Wrapper für System, Storage, GUI und Property; keine vollständige App-RPC-Werkzeugschicht. |
| Q02 | [backend/simpleserialoperation.cpp](https://github.com/JoKeks2023/uFlipper/blob/1d26683ff5b751f219a4e40443b7c8355eb39469/backend/simpleserialoperation.cpp) | Antwort sammeln bis zum Endtoken; kein fertiger unbegrenzter Live-Datenpfad. |
| Q03 | [backend/flipperzero/rpc/startrpcoperation.cpp](https://github.com/JoKeks2023/uFlipper/blob/1d26683ff5b751f219a4e40443b7c8355eb39469/backend/flipperzero/rpc/startrpcoperation.cpp) | Übergang CLI → RPC über start_rpc_session. |
| Q04 | [backend/flipperzero/helper/deviceinfohelper.cpp](https://github.com/JoKeks2023/uFlipper/blob/1d26683ff5b751f219a4e40443b7c8355eb39469/backend/flipperzero/helper/deviceinfohelper.cpp) | Versionsabhängige Geräteabfrage mit Property-/Legacy-Pfad. |
| Q05 | [backend/flipperzero/deviceinfo.h](https://github.com/JoKeks2023/uFlipper/blob/1d26683ff5b751f219a4e40443b7c8355eb39469/backend/flipperzero/deviceinfo.h) | Bisheriges Geräte-/Firmware-/Speichermodell. |
| Q06 | [backend/flipperzero/screenstreamer.cpp](https://github.com/JoKeks2023/uFlipper/blob/1d26683ff5b751f219a4e40443b7c8355eb39469/backend/flipperzero/screenstreamer.cpp) | Bildschirmstream und Eingaben, kein Messdatenstream. |
| Q07 | [plugins/flipperproto0/messages/application.pb.h](https://github.com/JoKeks2023/uFlipper/blob/1d26683ff5b751f219a4e40443b7c8355eb39469/plugins/flipperproto0/messages/application.pb.h) | Vorhandene generierte App-Nachrichten; gegen Momentum-Schema abzugleichen. |
| Q08 | [backend/flipperzero/utility/userbackupoperation.cpp](https://github.com/JoKeks2023/uFlipper/blob/1d26683ff5b751f219a4e40443b7c8355eb39469/backend/flipperzero/utility/userbackupoperation.cpp) | Backup-Verzeichnis /int; keine gesamte SD-Sicherung. |
| Q09 | [backend/flipperzero/toplevel/settingsbackupoperation.cpp](https://github.com/JoKeks2023/uFlipper/blob/1d26683ff5b751f219a4e40443b7c8355eb39469/backend/flipperzero/toplevel/settingsbackupoperation.cpp) | Nutzt backupInternalStorage. |
| Q10 | [backend/flipperzero/toplevel/settingsrestoreoperation.cpp](https://github.com/JoKeks2023/uFlipper/blob/1d26683ff5b751f219a4e40443b7c8355eb39469/backend/flipperzero/toplevel/settingsrestoreoperation.cpp) | Interne Wiederherstellung mit Neustart. |
| Q11 | [backend/flipperzero/utility/userrestoreoperation.cpp](https://github.com/JoKeks2023/uFlipper/blob/1d26683ff5b751f219a4e40443b7c8355eb39469/backend/flipperzero/utility/userrestoreoperation.cpp) | Dateibasierte Wiederherstellung, kein allgemeiner transaktionaler Snapshot. |
| Q12 | [backend/flipperzero/utility/directorydownloadoperation.cpp](https://github.com/JoKeks2023/uFlipper/blob/1d26683ff5b751f219a4e40443b7c8355eb39469/backend/flipperzero/utility/directorydownloadoperation.cpp) | Baustein für Verzeichnis-Download. |
| Q13 | [backend/flipperzero/toplevel/firmwareinstalloperation.cpp](https://github.com/JoKeks2023/uFlipper/blob/1d26683ff5b751f219a4e40443b7c8355eb39469/backend/flipperzero/toplevel/firmwareinstalloperation.cpp) | DFU-/Recovery-Pfad und temporäre interne Sicherung. |
| Q14 | [backend/flipperzero/toplevel/fullupdateoperation.cpp](https://github.com/JoKeks2023/uFlipper/blob/1d26683ff5b751f219a4e40443b7c8355eb39469/backend/flipperzero/toplevel/fullupdateoperation.cpp) | TGZ-/SD-Updater; kein dauerhafter vollständiger Benutzerbackup-Schritt im Ablauf. |
| Q15 | [backend/remotefilefetcher.cpp](https://github.com/JoKeks2023/uFlipper/blob/1d26683ff5b751f219a4e40443b7c8355eb39469/backend/remotefilefetcher.cpp) | SHA256-Vergleich bei vorhandener erwarteter Prüfsumme. |
| Q16 | [backend/versioninfo.cpp](https://github.com/JoKeks2023/uFlipper/blob/1d26683ff5b751f219a4e40443b7c8355eb39469/backend/versioninfo.cpp) | Numerische Versionen; keine universelle Ordnung verschiedener Firmware-Projekte. |
| Q17 | [application/application.cpp](https://github.com/JoKeks2023/uFlipper/blob/1d26683ff5b751f219a4e40443b7c8355eb39469/application/application.cpp) | Bestehende qFlipper-App-Updateadresse. |

## Momentum-Belege

Die Dateinamen sind die stabilen Zuordnungen der Funktionsmatrix. Alle Links sind auf denselben Commit festgelegt.

| ID | Quelldatei |
| --- | --- |
| M-IR | [applications/main/infrared/infrared_cli.c](https://github.com/Next-Flip/Momentum-Firmware/blob/e1784e7418d8b074e971983ceb6fef0f37e52ae4/applications/main/infrared/infrared_cli.c) |
| M-RF | [applications/main/subghz/subghz_cli.c](https://github.com/Next-Flip/Momentum-Firmware/blob/e1784e7418d8b074e971983ceb6fef0f37e52ae4/applications/main/subghz/subghz_cli.c) |
| M-LF | [applications/main/lfrfid/lfrfid_cli.c](https://github.com/Next-Flip/Momentum-Firmware/blob/e1784e7418d8b074e971983ceb6fef0f37e52ae4/applications/main/lfrfid/lfrfid_cli.c) |
| M-IB | [applications/main/ibutton/ibutton_cli.c](https://github.com/Next-Flip/Momentum-Firmware/blob/e1784e7418d8b074e971983ceb6fef0f37e52ae4/applications/main/ibutton/ibutton_cli.c) |
| M-1W | [applications/main/onewire/onewire_cli.c](https://github.com/Next-Flip/Momentum-Firmware/blob/e1784e7418d8b074e971983ceb6fef0f37e52ae4/applications/main/onewire/onewire_cli.c) |
| M-GPIOCLI | [applications/services/cli/cli_command_gpio.c](https://github.com/Next-Flip/Momentum-Firmware/blob/e1784e7418d8b074e971983ceb6fef0f37e52ae4/applications/services/cli/cli_command_gpio.c) |
| M-GPIORPC | [applications/services/rpc/rpc_gpio.c](https://github.com/Next-Flip/Momentum-Firmware/blob/e1784e7418d8b074e971983ceb6fef0f37e52ae4/applications/services/rpc/rpc_gpio.c) |
| M-PROP | [applications/services/rpc/rpc_property.c](https://github.com/Next-Flip/Momentum-Firmware/blob/e1784e7418d8b074e971983ceb6fef0f37e52ae4/applications/services/rpc/rpc_property.c) |
| M-SYS | [applications/services/rpc/rpc_system.c](https://github.com/Next-Flip/Momentum-Firmware/blob/e1784e7418d8b074e971983ceb6fef0f37e52ae4/applications/services/rpc/rpc_system.c) |
| M-APP | [applications/services/rpc/rpc_app.c](https://github.com/Next-Flip/Momentum-Firmware/blob/e1784e7418d8b074e971983ceb6fef0f37e52ae4/applications/services/rpc/rpc_app.c) |
| M-GUI | [applications/services/rpc/rpc_gui.c](https://github.com/Next-Flip/Momentum-Firmware/blob/e1784e7418d8b074e971983ceb6fef0f37e52ae4/applications/services/rpc/rpc_gui.c) |
| M-STOR | [applications/services/rpc/rpc_storage.c](https://github.com/Next-Flip/Momentum-Firmware/blob/e1784e7418d8b074e971983ceb6fef0f37e52ae4/applications/services/rpc/rpc_storage.c) |
| M-LOAD | [applications/services/loader/loader_cli.c](https://github.com/Next-Flip/Momentum-Firmware/blob/e1784e7418d8b074e971983ceb6fef0f37e52ae4/applications/services/loader/loader_cli.c) |
| M-POWER | [applications/services/power/power_cli.c](https://github.com/Next-Flip/Momentum-Firmware/blob/e1784e7418d8b074e971983ceb6fef0f37e52ae4/applications/services/power/power_cli.c) |
| M-BT | [applications/services/bt/bt_cli.c](https://github.com/Next-Flip/Momentum-Firmware/blob/e1784e7418d8b074e971983ceb6fef0f37e52ae4/applications/services/bt/bt_cli.c) |
| M-UARTAPP | [applications/main/gpio/application.fam](https://github.com/Next-Flip/Momentum-Firmware/blob/e1784e7418d8b074e971983ceb6fef0f37e52ae4/applications/main/gpio/application.fam) |
| M-UARTCLOSE | [applications/main/gpio/scenes/gpio_scene_usb_uart_close_rpc.c](https://github.com/Next-Flip/Momentum-Firmware/blob/e1784e7418d8b074e971983ceb6fef0f37e52ae4/applications/main/gpio/scenes/gpio_scene_usb_uart_close_rpc.c) |
| M-BADAPP | [applications/main/bad_usb/application.fam](https://github.com/Next-Flip/Momentum-Firmware/blob/e1784e7418d8b074e971983ceb6fef0f37e52ae4/applications/main/bad_usb/application.fam) |
| M-BAD | [applications/main/bad_usb/bad_usb_app.c](https://github.com/Next-Flip/Momentum-Firmware/blob/e1784e7418d8b074e971983ceb6fef0f37e52ae4/applications/main/bad_usb/bad_usb_app.c) |
| M-JS | [applications/system/js_app/js_app.c](https://github.com/Next-Flip/Momentum-Firmware/blob/e1784e7418d8b074e971983ceb6fef0f37e52ae4/applications/system/js_app/js_app.c) |
| M-JSTHREAD | [applications/system/js_app/js_thread.c](https://github.com/Next-Flip/Momentum-Firmware/blob/e1784e7418d8b074e971983ceb6fef0f37e52ae4/applications/system/js_app/js_thread.c) |
| M-SERIAL | [applications/system/js_app/modules/js_serial.c](https://github.com/Next-Flip/Momentum-Firmware/blob/e1784e7418d8b074e971983ceb6fef0f37e52ae4/applications/system/js_app/modules/js_serial.c) |
| M-GPIOJS | [applications/system/js_app/modules/js_gpio.c](https://github.com/Next-Flip/Momentum-Firmware/blob/e1784e7418d8b074e971983ceb6fef0f37e52ae4/applications/system/js_app/modules/js_gpio.c) |
| M-I2C | [applications/system/js_app/modules/js_i2c.c](https://github.com/Next-Flip/Momentum-Firmware/blob/e1784e7418d8b074e971983ceb6fef0f37e52ae4/applications/system/js_app/modules/js_i2c.c) |
| M-SPI | [applications/system/js_app/modules/js_spi.c](https://github.com/Next-Flip/Momentum-Firmware/blob/e1784e7418d8b074e971983ceb6fef0f37e52ae4/applications/system/js_app/modules/js_spi.c) |
| M-RFJS | [applications/system/js_app/modules/js_subghz/js_subghz.c](https://github.com/Next-Flip/Momentum-Firmware/blob/e1784e7418d8b074e971983ceb6fef0f37e52ae4/applications/system/js_app/modules/js_subghz/js_subghz.c) |
| M-BEACON | [applications/system/js_app/modules/js_blebeacon.c](https://github.com/Next-Flip/Momentum-Firmware/blob/e1784e7418d8b074e971983ceb6fef0f37e52ae4/applications/system/js_app/modules/js_blebeacon.c) |
| M-DISK | [applications/system/js_app/modules/js_usbdisk/js_usbdisk.c](https://github.com/Next-Flip/Momentum-Firmware/blob/e1784e7418d8b074e971983ceb6fef0f37e52ae4/applications/system/js_app/modules/js_usbdisk/js_usbdisk.c) |
| M-NFCCLI | [applications/main/nfc/cli/nfc_cli_commands.c](https://github.com/Next-Flip/Momentum-Firmware/blob/e1784e7418d8b074e971983ceb6fef0f37e52ae4/applications/main/nfc/cli/nfc_cli_commands.c) |
| M-NFSCAN | [applications/main/nfc/cli/commands/nfc_cli_command_scanner.c](https://github.com/Next-Flip/Momentum-Firmware/blob/e1784e7418d8b074e971983ceb6fef0f37e52ae4/applications/main/nfc/cli/commands/nfc_cli_command_scanner.c) |
| M-NFDUMP | [applications/main/nfc/cli/commands/dump/nfc_cli_command_dump.c](https://github.com/Next-Flip/Momentum-Firmware/blob/e1784e7418d8b074e971983ceb6fef0f37e52ae4/applications/main/nfc/cli/commands/dump/nfc_cli_command_dump.c) |
| M-APDU | [applications/main/nfc/cli/commands/apdu/nfc_cli_command_apdu.c](https://github.com/Next-Flip/Momentum-Firmware/blob/e1784e7418d8b074e971983ceb6fef0f37e52ae4/applications/main/nfc/cli/commands/apdu/nfc_cli_command_apdu.c) |
| M-NFRAW | [applications/main/nfc/cli/commands/raw/nfc_cli_command_raw.c](https://github.com/Next-Flip/Momentum-Firmware/blob/e1784e7418d8b074e971983ceb6fef0f37e52ae4/applications/main/nfc/cli/commands/raw/nfc_cli_command_raw.c) |
| M-MFREAD | [applications/main/nfc/cli/commands/mfu/nfc_cli_action_rdbl.c](https://github.com/Next-Flip/Momentum-Firmware/blob/e1784e7418d8b074e971983ceb6fef0f37e52ae4/applications/main/nfc/cli/commands/mfu/nfc_cli_action_rdbl.c) |
| M-MFWRITE | [applications/main/nfc/cli/commands/mfu/nfc_cli_action_wrbl.c](https://github.com/Next-Flip/Momentum-Firmware/blob/e1784e7418d8b074e971983ceb6fef0f37e52ae4/applications/main/nfc/cli/commands/mfu/nfc_cli_action_wrbl.c) |
| M-NFEMU | [applications/main/nfc/cli/commands/nfc_cli_command_emulate.c](https://github.com/Next-Flip/Momentum-Firmware/blob/e1784e7418d8b074e971983ceb6fef0f37e52ae4/applications/main/nfc/cli/commands/nfc_cli_command_emulate.c) |
| M-IRRPC | [applications/main/infrared/scenes/infrared_scene_rpc.c](https://github.com/Next-Flip/Momentum-Firmware/blob/e1784e7418d8b074e971983ceb6fef0f37e52ae4/applications/main/infrared/scenes/infrared_scene_rpc.c) |
| M-RFRPC | [applications/main/subghz/scenes/subghz_scene_rpc.c](https://github.com/Next-Flip/Momentum-Firmware/blob/e1784e7418d8b074e971983ceb6fef0f37e52ae4/applications/main/subghz/scenes/subghz_scene_rpc.c) |
| M-LFRPC | [applications/main/lfrfid/scenes/lfrfid_scene_rpc.c](https://github.com/Next-Flip/Momentum-Firmware/blob/e1784e7418d8b074e971983ceb6fef0f37e52ae4/applications/main/lfrfid/scenes/lfrfid_scene_rpc.c) |
| M-IBRPC | [applications/main/ibutton/scenes/ibutton_scene_rpc.c](https://github.com/Next-Flip/Momentum-Firmware/blob/e1784e7418d8b074e971983ceb6fef0f37e52ae4/applications/main/ibutton/scenes/ibutton_scene_rpc.c) |
| M-NFRPC | [applications/main/nfc/helpers/protocol_support/nfc_protocol_support.c](https://github.com/Next-Flip/Momentum-Firmware/blob/e1784e7418d8b074e971983ceb6fef0f37e52ae4/applications/main/nfc/helpers/protocol_support/nfc_protocol_support.c) |
| M-CCIDFAM | [applications/debug/ccid_test/application.fam](https://github.com/Next-Flip/Momentum-Firmware/blob/e1784e7418d8b074e971983ceb6fef0f37e52ae4/applications/debug/ccid_test/application.fam) |
| M-SHELL | [applications/services/cli/cli_main_shell.c](https://github.com/Next-Flip/Momentum-Firmware/blob/e1784e7418d8b074e971983ceb6fef0f37e52ae4/applications/services/cli/cli_main_shell.c) |
| M-CCID | [applications/debug/ccid_test/ccid_test_app.c](https://github.com/Next-Flip/Momentum-Firmware/blob/e1784e7418d8b074e971983ceb6fef0f37e52ae4/applications/debug/ccid_test/ccid_test_app.c) |
| M-CCIDCMD | [applications/debug/ccid_test/ccid_test_app_commands.c](https://github.com/Next-Flip/Momentum-Firmware/blob/e1784e7418d8b074e971983ceb6fef0f37e52ae4/applications/debug/ccid_test/ccid_test_app_commands.c) |
| M-UART | [applications/main/gpio/usb_uart_bridge.c](https://github.com/Next-Flip/Momentum-Firmware/blob/e1784e7418d8b074e971983ceb6fef0f37e52ae4/applications/main/gpio/usb_uart_bridge.c) |
| M-PROPDEF | [lib/toolbox/property.h](https://github.com/Next-Flip/Momentum-Firmware/blob/e1784e7418d8b074e971983ceb6fef0f37e52ae4/lib/toolbox/property.h) |
| M-POWERHAL | [targets/f7/furi_hal/furi_hal_power.c](https://github.com/Next-Flip/Momentum-Firmware/blob/e1784e7418d8b074e971983ceb6fef0f37e52ae4/targets/f7/furi_hal/furi_hal_power.c) |
| M-CCIDHAL | [targets/f7/furi_hal/furi_hal_usb_ccid.c](https://github.com/Next-Flip/Momentum-Firmware/blob/e1784e7418d8b074e971983ceb6fef0f37e52ae4/targets/f7/furi_hal/furi_hal_usb_ccid.c) |
| M-BADWORK | [applications/main/bad_usb/scenes/bad_usb_scene_work.c](https://github.com/Next-Flip/Momentum-Firmware/blob/e1784e7418d8b074e971983ceb6fef0f37e52ae4/applications/main/bad_usb/scenes/bad_usb_scene_work.c) |
| M-CMD | [applications/services/cli/cli_main_commands.c](https://github.com/Next-Flip/Momentum-Firmware/blob/e1784e7418d8b074e971983ceb6fef0f37e52ae4/applications/services/cli/cli_main_commands.c) |

## Ergänzende Primärquellen

| ID | Quelle | Verwendung |
| --- | --- | --- |
| W01 | [IR-Empfänger und Demodulation](https://docs.flipper.net/zero/infrared/read) | Demodulierte IR-Empfangsdaten; kein optisches Amplitudenspektrum. |
| W02 | [Sub-GHz-Hardware](https://docs.flipper.net/zero/sub-ghz) | CC1101 und unterstützte Frequenzbereiche; kein universeller SDR. |
| W03 | [Momentum mntm-012 Release](https://github.com/Next-Flip/Momentum-Firmware/releases/tag/mntm-012) | Release-Metadaten und Update-Paket, heute keine Installation. |
| W04 | [Offizielles Firmware-Verzeichnis](https://update.flipperzero.one/firmware/directory.json) | Kanäle release, release-candidate, development am Recherchetag. |
| W05 | [Apple Smartcard-Entitlement](https://developer.apple.com/documentation/bundleresources/entitlements/com.apple.security.smartcard) | Zugriff sandboxierter Reader-Clients; implementiert keinen eigenen Reader. |
| W06 | [Apple Smartcard-Slot-Namen](https://developer.apple.com/de/documentation/cryptotokenkit/tksmartcardslotmanager/slotnames) | Betriebssystemseitig sichtbare Slots als Abnahmepunkt. |
| W07 | [Apple Smartcard-Slot-Zustände](https://developer.apple.com/documentation/cryptotokenkit/tksmartcardslot/state-swift.enum?language=objc) | Missing/empty/probing/mute/valid als Integrationsfälle. |
| W08 | [vsmartcard-Projektdokumentation](https://github.com/frankmorgner/vsmartcard/blob/master/virtualsmartcard/doc/README.txt.in) | Virtueller PC/SC-Reader als Kandidat; historische macOS-Hinweise sind keine aktuelle OS-Zertifizierung. |
| W09 | [Google Drive App-Daten](https://developers.google.com/workspace/drive/api/guides/appdata) | Google Drive ist als Release-Anbieter gewählt; App-Datenbereich ist ein Integrationsansatz, noch keine fertige Anbindung. |
| W10 | [Dropbox Entwicklerleitfaden](https://docs.dropboxapi.com/dropbox-api/docs/developer-resources/developer-guide) | Dropbox ist als Release-Anbieter gewählt; App-Registrierung, Berechtigungen und Integration noch zu prüfen. |
| W11 | [Microsoft Graph App-Ordner](https://learn.microsoft.com/en-us/graph/onedrive-sharepoint-appfolder) | OneDrive ist als Release-Anbieter gewählt; AppFolder ist ein Integrationsansatz, Anbietergrenzen noch zu prüfen. |
| W12 | [Nextcloud WebDAV](https://docs.nextcloud.com/server/latest/developer_manual/client_apis/WebDAV/basic.html) | WebDAV ist als Release-Anbindung gewählt; konkrete Server und Konfliktpfad separat testen. |
| W13 | [S3 Conditional Writes](https://docs.aws.amazon.com/AmazonS3/latest/userguide/conditional-writes.html) | S3 ist als Release-Anbindung gewählt; bedingte Writes und kompatible Server jeweils prüfen. |

## Besonders wichtige Fundstellen

- Q01 Zeilen 69–97: Host-Operationen. Q07 und M-APP müssen vor neuen App-RPC-Aufrufen auf Schema-/Versionskompatibilität abgeglichen werden.
- Q02 Zeilen 26–38: Antwort wird im Speicher gesammelt. Daraus folgt der Bedarf eines eigenen begrenzten Streaming-Pfads; kein gemessener Speicherfehler behauptet.
- Q08 Zeilen 18–25: `/int` als Sicherungswurzel. Q09 Zeile 45: interne Sicherung. Q14 Zeilen 57–103: Ablauf ohne vorgeschalteten dauerhaften vollständigen Benutzerbackup.
- M-IR: `ir rx [raw]` und Callback mit dekodierten Werten bzw. Pulszeiten. W01 begrenzt die Interpretation.
- M-RF Zeilen 393–465: `rx_raw` liefert vorzeichenbehaftete Dauern; Reset-/Fehlermarker erhalten. Nicht mit I/Q-Daten gleichsetzen.
- M-APDU Zeilen 190–222: APDU-Liste wird innerhalb eines Aufrufs abgearbeitet, danach Poller beendet. Kein Nachweis einer dauerhaft vom Mac gesteuerten Kartensitzung.
- M-CCIDFAM/M-CCID/M-CCIDCMD: CCID-Debugtest mit Beispielantworten. Kein Nachweis, dass ein Release-Gerät damit seine NFC-Antenne als Systemreader bereitstellt.
- M-JS Zeilen 135–211: JS-CLI und Ausgabe über Pipe. M-SERIAL/M-GPIOJS/M-I2C/M-SPI/M-RFJS: Scriptmodule als Kandidaten für ausdrücklich freigegebene Hilfsprogramme.
- M-UARTCLOSE und M-UART: USB-UART-Modus beendet RPC bzw. kann das USB-Profil ändern. Wiederverbindung gehört zum Funktionsnachweis.
- M-PROP Zeilen 11–13 und 78–83: `devinfo`, `pwrinfo`, `pwrdebug` über RPC-Properties.

Aus Quellcode abgeleitete Integrationswege sind ausdrücklich Vorschläge. Antwortzeiten, Durchsatz, langfristige Stabilität, FAP-ABI, vollständige Kartenabdeckung und Betriebssystemintegration sind damit nicht belegt.
