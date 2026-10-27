# Laufzeit-/Validierungsnachweise

Datum: 27.10.2026. Dies ist ein Zwischenstand, keine Fertigmeldung.

## Lokal tatsächlich ausgeführt

- Kleiner nativer C++17-Build mit Apple Clang 21, SQLite3 3.54 und libsodium 1.0.22; zwei Compilerprozesse, kein Qt-Build/Simulator.
- CTest `core_contract`: bestanden. Fälle: kein Start ohne Gerät/Aufnahme, UI/MCP-Konflikt ohne stillen Stop, Warten auf Stop-Bestätigung, verweigerter Wechsel, Verbindungsverlust/Generationswechsel, Belegung bleibt bei unbekanntem Stop-Ausgang erhalten.
- Echte Datei-/SQLite-Abläufe: 6,4 MB Originaldaten fortlaufend geschrieben, Manifest/Katalog geöffnet, Hashprüfung, Korruption erkannt, Prozessabbruch ohne Destruktor und Wiederauffinden als unterbrochen.
- Echte libsodium-Dateiver-/entschlüsselung mit Datenvergleich; falscher Schlüssel und abgeschnittener Stream abgewiesen, keine unvollständige Klartextausgabe veröffentlicht; Passwortableitung und Recovery-Code-Rundlauf.
- Angeschlossener Flipper lesend geprüft: Firmware `mntm-012`, Commit `e1784e74`, RPC `0.25`, Firmware-API `87.1`. Ein initialer `power_info`-Versuch wurde von Momentum als unbekannt zurückgewiesen; Powerdaten werden anschließend über die belegte RPC-Property-Schnittstelle integriert. Keine Power-Einstellung geändert.

## Noch offen

Qt-App-Build/Start, gemeinsame Nutzung der echten Verbindung, Live-Aufnahmen aus der App, Restore, Helferinstallation, NFC-Systemreader, Cloud-Anmeldung/-Synchronisierung und MCP-Clienttest sind noch nicht nachgewiesen. CI-Ergebnisse werden nach dem tatsächlichen Lauf ergänzt. Kein öffentlicher Release erfolgt.

## Zusätzliche beobachtete Ergebnisse

- CTest `serial_contract` mit echter Pseudoterminal-Verbindung: Originalbytes, über mehrere Reads geteilte CLI-Stopbestätigung und Timeout bestanden.
- Echter Flipper: fünf Sekunden `ir rx raw` ausgeführt; 66 empfangene Bytes, Stop durch CLI-Prompt bestätigt. Aufnahme `86e91731e855237eb67d54f634c53080` dauerhaft in der lokalen App-Ablage gespeichert und Integrität geprüft. **0 IR-Frames empfangen**; dies belegt Start/Stop/Ablage, noch keinen echten Pulsdatenempfang.
- GitHub CI Commit `6605ddf`: Ubuntu-Coreprüfung erfolgreich; Desktop-Build noch fehlgeschlagen und macOS-Jobs wartend. Fehlerursache beim Qt-Pfad korrigiert; neuer Lauf folgt. Kein grüner Gesamtbuild behauptet.
- Visuelle Entwürfe lokal gerendert und betrachtet; verbleibende Nutzerfreigabe dokumentiert. Kein öffentlicher Release.
