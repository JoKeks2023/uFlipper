# Laufzeit-/Validierungsnachweise

Datum: 27.10.2026. Dies ist ein Zwischenstand, keine Fertigmeldung.

## Lokal tatsächlich ausgeführt

- Kleiner nativer C++17-Build mit Apple Clang 21, SQLite3 3.54 und libsodium 1.0.22; zwei Compilerprozesse, kein Qt-Build/Simulator.
- CTest `core_contract`: bestanden. Fälle: kein Start ohne Gerät/Aufnahme, UI/MCP-Konflikt ohne stillen Stop, Warten auf Stop-Bestätigung, verweigerter Wechsel, Verbindungsverlust/Generationswechsel, Belegung bleibt bei unbekanntem Stop-Ausgang erhalten.
- Echte Datei-/SQLite-Abläufe: 6,4 MB Originaldaten fortlaufend geschrieben, Manifest/Katalog geöffnet, Hashprüfung, Korruption erkannt, Prozessabbruch ohne Destruktor und Wiederauffinden als unterbrochen.
- Echte libsodium-Dateiver-/entschlüsselung mit Datenvergleich; falscher Schlüssel und abgeschnittener Stream abgewiesen, keine unvollständige Klartextausgabe veröffentlicht; Passwortableitung und Recovery-Code-Rundlauf.
- Angeschlossener Flipper lesend geprüft: Firmware `mntm-012`, Commit `e1784e74`, RPC `0.25`, Firmware-API `87.1`. Ein initialer `power_info`-Versuch wurde von Momentum als unbekannt zurückgewiesen; Powerdaten werden anschließend über die belegte RPC-Property-Schnittstelle integriert. Keine Power-Einstellung geändert.

## Noch offen

Aktueller Stand siehe die datierten Nachweise weiter unten. Noch offen sind insbesondere der eigenständige Mac-Paketstart, gemeinsame Nutzung der echten Verbindung durch UI/MCP/Reader, Live-Aufnahmen aus der App, vollständiges Gerätebackup und Produkt-Restore, NFC-Kartensitzung/Systemreader, Cloud-Anmeldung/-Synchronisierung und MCP-Clienttest. Kein öffentlicher Release erfolgt.

## Zusätzliche beobachtete Ergebnisse

- CTest `serial_contract` mit echter Pseudoterminal-Verbindung: Originalbytes, über mehrere Reads geteilte CLI-Stopbestätigung und Timeout bestanden.
- Echter Flipper: fünf Sekunden `ir rx raw` ausgeführt; 66 empfangene Bytes, Stop durch CLI-Prompt bestätigt. Aufnahme `86e91731e855237eb67d54f634c53080` dauerhaft in der lokalen App-Ablage gespeichert und Integrität geprüft. **0 IR-Frames empfangen**; dies belegt Start/Stop/Ablage, noch keinen echten Pulsdatenempfang.
- GitHub CI Commit `6605ddf`: Ubuntu-Coreprüfung erfolgreich; Desktop-Build noch fehlgeschlagen und macOS-Jobs wartend. Fehlerursache beim Qt-Pfad korrigiert; neuer Lauf folgt. Kein grüner Gesamtbuild behauptet.
- Visuelle Entwürfe lokal gerendert und betrachtet; verbleibende Nutzerfreigabe dokumentiert. Kein öffentlicher Release.

## Nächste beobachtete Prüfungen

- CI `c3b0d03`, Run `40530798550`: kompletter Linux-Desktop-Build und Core auf Ubuntu 22.04 sowie Intel-macOS erfolgreich. ARM-macOS-Jobs wartend; kein macOS-Desktop-Erfolg behauptet.
- Echter IR-Empfang mit neuer Startprüfung: Aufnahme `4cc4595a480883148b26de95acb3aabe`, 66 Bytes, 0 Signalframes; Start und Stop bestätigt, Integrität geprüft.
- Echter Sub-GHz-Protokollempfang, intern, 433,92 MHz: Aufnahme `7ac8f577d39a176e7bc429246e9a88c7`, 214 Bytes; Start/Stop und Integrität bestätigt. Kein Protokolldekodier-Nachweis abgeleitet.
- Echter Sub-GHz-Pulsempfang, 433,92 MHz: Aufnahme `00f3ffa8d6db2d558df4a784e6a90313`, 99 Bytes; Start/Stop und Integrität bestätigt. Keine gezielten externen Pulse als nachgewiesen markiert.
- Mit Nutzer abgestimmter 20-Sekunden-IR-Test: Aufnahme `3f9b9f32aedf3652020ee0679e0a0b06`, erneut 0 Signalframes. Nutzer wünscht erneuten Signaltest später. Keine Aussage über tatsächliche Pulsauswertung möglich.
- Katalogregression: zunächst roter Test `Cannot create output file` nach entferntem Aufnahmeordner; nach Korrektur beide CTest-Suiten grün. Intakte Aufnahme bleibt lesbar, fehlende Aufnahme bleibt sichtbar und unverifiziert.
- NFC-Helfer lokal erfolgreich gebaut, SDK-Ziel f7 / API 87.1. Dies ist ausschließlich Build-Nachweis; Installation, dauerhafte Kartensitzung und PC/SC bleiben offen.
- Qt-App-RPC-Vertragstests ergänzt und für nächsten CI-Lauf vorgesehen; lokal fehlt Qt. Noch kein Ausführungsergebnis für diese Tests.

## CI-Abschluss und weitere lokale Kernprüfungen

- **Run 40533899143, Commit 8626b3c: alle vier Jobs erfolgreich.** Ubuntu- und Intel-macOS-Core, kompletter Linux-Desktop sowie macOS-Universal-Paketbau, jeweils einschließlich App-RPC-Vertragstests. Mac-Deployment-Ziel weiterhin 13; keine Zertifizierung eines gestarteten Pakets aus diesem Ergebnis abgeleitet.
- Backup-Kern lokal tatsächlich mit Dateien/SQLite ausgeführt: Vollgerät-Abschluss ohne beide Inventare blockiert; Änderung nach Inventarbestätigung verwirft Freigabe; leere Verzeichnisse erhalten; Traversal abgewiesen; Testsicherungen separat markiert; zu kurze Datei abgewiesen; Payload- und Katalogpfadkorruption erkannt.
- Echter Prozessabbruch während eines Backup-Dateitransfers: nach Wiederöffnen unterbrochen, nicht geprüft; Aufnahme-/Seriell-Vertragstests weiterhin bestanden. Letzter kleiner lokaler Build: 2/2 CTest-Suiten grün.
- Neu angeschlossene Qt-Telemetrie, Geräteprobe, temporäre Vollbackup-Sperre und native Paket-Starttests noch nicht ausgeführt; nächster CI-Lauf prüft diese Änderungen.

## Beobachteter Mac-Paketfehler

- Heruntergeladenes CI-Paket `8626b3c` tatsächlich auf Apple Silicon gestartet. `lipo` bestätigt **x86_64 arm64**; Bundle-ID bestätigt **com.jorisconrad.flipperzeroutil**.
- Erster Start durch macOS mit Signal 9 beendet. `codesign --verify --deep --strict`: **code object is not signed at all**; ursprüngliches Binary nur linker-signiert, Info.plist und Ressourcen nicht gebunden. Keine Sicherheitsregeln des Computers geändert.
- Lokal eigenes Testartefakt ad hoc nachsigniert und strikt geprüft; Ergebnis des erneuten Starts wird separat dokumentiert. Der CI-Build allein belegt keine laufende App.
- Nächster Paketbau signiert das vollständige Testbundle ausdrücklich ad hoc und prüft die Signatur. Developer-ID-Signierung/Notarisierung und Veröffentlichung bleiben offen.

- Nach lokaler Ad-hoc-Signierung startet das alte Mac-Paket bis zur QML-Ladeprüfung und endet mit `module "QtQuick.Window" is not installed`. Damit ist die fehlende QML-Paketierung tatsächlich beobachtet, nicht nur vermutet. Die QML-Deploymentkorrektur ist bereits im nächsten Buildpfad enthalten.
- CI `b955e82`, Run `40538788021`: beide Core-Jobs erfolgreich, Desktop-Builds scheitern am fehlenden DFU-Header-Suchpfad des neu hinzugefügten Geräteprüfprogramms. Suchpfad ergänzt; frühe isolierte Compilerprüfung für Probe/Telemetrie vor dem kompletten App-Build hinzugefügt. Kein neuer grüner Gesamtstand behauptet.

## Freigegebener Helfer: tatsächlicher Gerätetest

- Nutzer hat genau Helfer **0.1.0 / ab0bcae1…70e0d632** unter `/ext/apps/Tools/uf_nfc_bridge.fap` zur Installation und Start/Versions-/Stopprüfung freigegeben. Keine Freigabe eines Kartentests daraus abgeleitet.
- Datei mit Momentum-SDK-Storage übertragen, vollständig zurückgelesen und SHA-256 verglichen: identisch; keine vorhandene Helferdatei überschrieben. Installationsprotokoll dauerhaft in der lokalen App-Ablage.
- Kleinen nativen Prüfer mit der gepinnten nanopb-Version und den vorhandenen generierten Protobuf-Definitionen gebaut; kein lokaler Qt-Build. StartRequest, AppStarted-Broadcast, DataExchange-Info und AppExit samt AppClosed-Broadcast real ausgeführt.
- Erster Lauf: Helfer korrekt gestoppt; Rückkehr zu CLI zunächst nicht bestätigt. Aufnahme `2dae9a886485b0abfa91a155e7d33d3b` als unterbrochen erhalten. Momentum `rpc_cli.c` wartet nach dem Stop-Callback auf das nächste Transportbyte; Client nach bestätigtem RPC-Abschluss entsprechend angepasst. Quelle: https://github.com/Next-Flip/Momentum-Firmware/blob/e1784e7418d8b074e971983ceb6fef0f37e52ae4/applications/services/rpc/rpc_cli.c
- Zweiter Lauf: **Start, Info `uf-nfc/0.1.0;iso14443-4a;short-apdu;max=240`, Stop und RPC-Abschluss bestätigt.** Aufnahme `a437c80e5fac071fa4f281c9fee75f54` abgeschlossen und Integrität geprüft. Keine Kartenaktivierung durchgeführt.
- CI `f3727c6`: frühe Linux-Compilerprüfung erkennt mehrdeutige Qt-Kommandozeilenoptions-Initialisierung; durch explizite Konstruktoren korrigiert. Nächster CI-Lauf steht noch aus.

## Backup-/Restore-Test am angeschlossenen Flipper

- Kleines natives Testprogramm `uflipper_backup_fixture_probe` mit bestehendem RPC-Transport und `BackupStore` gebaut; beide lokalen CTest-Suiten bestanden. Der Test erlaubt nur einen frisch angelegten zufälligen Ordner unter `/ext/uflipper-tests/`.
- Tatsächlich ausgeführt: zwei neue Dateien (Text und 3.072 Binärbytes einschließlich NUL) plus leeres Verzeichnis angelegt; Inhalte über Storage-RPC zurückgelesen und als **TestSubset** lokal gesichert, Manifest/Payloads geprüft. Originalbackup: `471033dc4d7638738addfdc33415b26e`.
- Beide Gerätedateien verändert; vor Restore den geänderten Stand erneut gelesen und als zweite geprüfte Testsicherung gespeichert: `ebffac75238c52629dd5db4501df4e71`.
- Originale aus den gesicherten Payloads zurückgeschrieben; beide Dateien erneut vom Flipper gelesen und **bytegenau mit den Originaldaten verglichen**. Anschließend ausschließlich den eigenen Testordner entfernt, Nichtvorhandensein per Storage-Stat bestätigt und RPC sauber zur CLI geschlossen.
- Gerätepfad: `/ext/uflipper-tests/c602b5862a9a4e8863830f568427117f`; dauerhafte Ablaufaufnahme `9883e2f9ff5962ef4bc8eab3522b50d1`, Integrität geprüft. Keine bestehenden Nutzerdateien verändert.
- Dies belegt den begrenzten Geräte-Rundlauf. **Kein Nachweis eines vollständigen Gerätesicherungs- oder Produkt-Restore-Workflows**; die temporäre Wartungssperre bleibt bestehen.

## Verfügbare Testmittel und aktueller Build

- Nutzer: derzeit keine passende ISO-DEP/Typ-4A-NDEF-Testkarte. Dauerhafte NFC-Kartensitzung und unabhängiger Mac-PC/SC-Test bleiben an dieser Voraussetzung offen. Release-Umfang bleibt erhalten; keine Firmwareänderung und kein anderer Kartentest als Ersatz.
- Nutzer hat IR-Signaltest auf später verschoben; kein erneuter Empfangslauf gestartet.
- Designentwürfe weiterhin nicht zur Umsetzung freigegeben: Nutzer konnte sie nicht ansehen. Große UI-Änderungen bleiben ausgesetzt.
- CI `290f6ce`, Run `40561302551`: beide Core-Jobs sowie kompletter Linux-Desktop, Qt-Vertragstests und nativer Linux-App-Start erfolgreich. Mac-Universal-Paket noch im Bau; Ergebnis folgt nach tatsächlichem Abschluss.

- Abschluss CI `290f6ce` / Run `40561302551`: **alle vier Jobs erfolgreich**, einschließlich macOS-Universal-Build, Qt-App-RPC-Verträgen und gebündeltem Mac-App-Start bei verborgenem Entwickler-Qt. Eigenständiger Start dieses Pakets auf dem lokalen Apple-Silicon-Mac und Power-Abfrage folgen separat.

## Eigenständiges Mac-Paket und Qt-Gerätenachweise

- CI-Artefakt `290f6ce` heruntergeladen und ohne Nachsignieren/OS-Ausnahmen entpackt. Vollständiges Bundle besteht `codesign --verify --deep --strict`; Hauptprogramm enthält `x86_64 arm64`, Bundle-ID ist `com.jorisconrad.flipperzeroutil`.
- Paket tatsächlich auf **Apple Silicon / macOS 27.0** ohne Entwickler-Qt gestartet: Exit 0 nach eingebautem Starttest. QML lädt; Flipper mit Momentum wird erkannt. Zusammen mit dem Intel-CI-Start ist dies ein praktischer Nachweis auf beiden Mac-Architekturen, **kein Laufzeitnachweis auf macOS 13**.
- Gebündeltes `uFlipper-probe` tatsächlich gegen den Flipper ausgeführt: Exit 0, `connected=true`, Powerformat 2.1 unterstützt, `available=true`, `stale=false`, kein Abfragefehler. Beobachtet am `2026-10-27T18:58:33.789Z`: Akku **99 %**, **21 °C**, **3.916 mV**, **0 mA**, Zustand `discharging`, Restkapazität 2.061 mAh, volle/designte Kapazität jeweils 2.100 mAh. Werte nicht als konstante Geräteeigenschaften behandeln.
- Derselbe Qt-Pfad mit `--helper`: Exit 0, Info 0.1.0 korrekt, Start und AppExit/AppClosed bestätigt, 0 APDUs. Keine Kartenaktivierung und keine erneute Helferinstallation. NFC-Kartensitzung/PCSC bleiben offen.
- **Beobachteter Nebeneffekt des geerbten Gerätestarts:** `VCPDeviceInfoHelper::syncTime()` führt nach dem Lesen der Uhr `System Set DateTime` aus. Die Power-Abfrage selbst liest nur; der gesamte Initialisierungspfad ist deshalb nicht vollständig schreibfrei. Dieses Verhalten wurde in diesem Schritt nicht geändert.
- Verbleibende beobachtete Startwarnungen: Timer aus fremdem Thread, QML-Variablenreihenfolge in `ConfirmationDialog`, zwei SVG-`use`-Elemente, ausstehende QML-Erstellung beim kurzen Test-Shutdown. Kein absturzfreier Produktbetrieb daraus abgeleitet. Logger schreibt aus dem Qt-Messagehandler direkt auf Timer/Objektzustand; Thread-Ursache vor Korrektur gezielt reproduzieren. Issue-Anlage weiterhin durch deaktivierte Fork-Issues/fehlende Connector-Schreibrechte blockiert; Befund bleibt in diesem Task.
- Maschinenlesbarer Beleg einschließlich Paket-Hash und Power-Rohwerten: `evidence/mac-runtime-290f6ce.json`. Vollständige lokale Start-/Probeprotokolle unter `/private/tmp/uflipper-runtime-290f6ce/` (nicht als dauerhafte Ablage voraussetzen).
- Neuester Code `8363e3c`, Run `40564637207`: beide Core-Jobs inklusive nativer Probe-Kompilation erfolgreich; Desktop-Läufe noch offen. Der oben beobachtete Paketstand bleibt eindeutig `290f6ce`.
