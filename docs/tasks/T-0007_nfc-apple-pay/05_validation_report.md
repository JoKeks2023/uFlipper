# Validierung

## Beobachteter Gerätelauf am 29.10.2026

- Zeit: **11:46:12.534–11:46:32.434 UTC**; Apple Silicon, macOS 27.0.1; Momentum `e1784e74`, unveränderter Helfer 0.1.0.
- Gegenstelle: vom Nutzer bereitgestelltes und freigegebenes Apple-Pay-iPhone.
- Nach wiederholter Erkennung ISO14443-4A aktiviert. Ohne erneute Aktivierung zwischen den Kommandos zweimal den festen SELECT-PPSE-Befehl gesendet. Beide Antworten: **46 Bytes, Statuswort `9000`**.
- Deaktivierung bestätigt; AppExit/AppClosed sowie RPC-Abschluss bestätigt. Prozess Exit 0.
- Aufnahme `92a914a1d85fc2ea8e13e9831dfb1fb0`: `completed`, **12.535 Bytes**, keine Lücken, SHA-256 mit Manifest verglichen. Originalbytes bleiben lokal unter dem ignorierten `build_mac/nfc-apple-pay-20261029/store/`. Keine Rohdaten im Git-Repo.
- Bisherigen Modus ohne NFC-Aktivierung anschließend erneut ausgeführt: Exit 0, Version/Start/Stop/RPC sowie Aufnahmeintegrität bestätigt; 0 Kartenkommandos.
- Kleinen CMake-Unterbau mit höchstens zwei parallelen Compilerprozessen gebaut. `core_contract` und `serial_contract`: bestanden. Kein lokaler Qt-Großbuild und kein Simulator.
- uFlipper nach dem Test wieder geöffnet.

Maschinenlesbare, auf Metadaten reduzierte Evidenz: [apple-pay-20261029.json](evidence/apple-pay-20261029.json).

## Aussagegrenzen

Belegt sind die NFC-Aktivierung und zwei APDU-Antworten innerhalb einer Aktivierung. Zwei identische SELECTs beweisen keine semantisch voneinander abhängigen Kartenkommandos. NDEF-Auslesen, Kartenwechsel/Abziehen, Reset-/Fehlerfälle und ein unabhängiger PC/SC-Client wurden in diesem Lauf nicht geprüft. Systemreader/Kernabnahme bleiben offen. Kein Ersatz der vereinbarten Gesamtprüfung und keine Veröffentlichung.

## Quellen

- [Flipper: EMV und Apple Pay](https://blog.flipper.net/rfid/)
- [Apple: NFC controller und Secure Element](https://support.apple.com/en-gb/guide/security/seccb53a35f0/web)
- [EMVCo Entry Point, Abschnitt 3.3.2: SELECT PPSE](https://ptacts.uspto.gov/ptacts/public-informations/petitions/1554468/download-documents?artifactId=nqKlyvjlPozNv24MUumo2TOF1tq4xGYq6plx1wEvj3gBFrtzEbJIErI)
