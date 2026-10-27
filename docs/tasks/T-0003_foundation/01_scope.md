# Verbindlicher Umfang und Reihenfolge

Grundlage: [27 Produktentscheidungen](../T-0001_product-plan/01_scope.md) und [65 Funktionen/Ziele](../T-0002_technical-plan/06_function_matrix.md). Der Nutzer hat den vollständigen finalen Plan ausdrücklich zur Umsetzung beauftragt.

1. Produktidentität, Mac-/Linux-CI und Abkopplung der Upstream-App-Updates.
2. Gemeinsamer Gerätebesitzer für UI/MCP/Reader, Auftragszustände, bestätigter Stop, Hardwarekonflikte.
3. SQLite-Katalog, dauerhaft gespeicherte Live-Sitzungen, begrenzter RAM, Zeit-/Verlustmodell.
4. Frühe Geräte-, Helfer-, NFC-Systemreader- und Backup-/Restore-Nachweise.
5. Visuelle Prüfung von Dashboard, aktivem Werkzeug und Settings vor größerem UI-Umbau; drei Layouts und Easy Mode.
6. Firmware-Bibliothek, vollständiger Backupper, Google Drive/Dropbox/OneDrive/WebDAV/S3, optionale Verschlüsselung per Schlüssel/Passwort, Papierkorb und MCP.
7. Vollständige Geräte-/OS-/Cloud-Prüfung und ausdrückliche Kernabnahme vor Release.

Die fünf letzten Entscheidungen: vollständige lesbare interne Daten und gesamte SD; Anmeldung/Entsperren/Pfade je Computer; macOS 13+ Universal und Ubuntu 22.04+ x86_64; Systemreader zunächst nur Mac; optionaler sichtbarer Hintergrundbetrieb.

Technik: C++17/Qt6/QML mit qmake für die App. Kleines CMake-Testziel für portable Kernmodule. SQLite-Katalog und Dateiaufnahmen. libsodium für authentifizierte Streams und Argon2id. MCP stdio mit 2026-07-28/2025-11-25. NFC zuerst FAP/App-RPC und lokale PC/SC-Bridge; OpenSC/ISO-DEP als Abnahme.
