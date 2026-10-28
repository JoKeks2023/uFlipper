# Umsetzung

Oberflächenbauschritt implementiert und nativ geprüft; abschließender Gerätenachweis des finalen Bundles noch offen. Freigabe der drei Entwürfe aus T-0003 liegt vor.

- Seitenleiste und resizierbare Arbeitsfläche, Echtwerte-Dashboard inklusive Speichergrößen, IR-/Werkzeugansicht, fünf Settings-Bereiche implementiert. Datei- und Bildschirmfunktionen eingebunden. Nicht angeschlossene Funktionen explizit gekennzeichnet.
- Aufnahme-/Backupkataloge werden lesend abgefragt; Testsicherungen als solche ausgewiesen. Easy Mode speichert die echte Einstellung. Navigation/Schließen während Dateiübertragung gesperrt.
- Logger leitet Nachrichten aus dem Renderthread in seinen Besitzerthread; gezielter Qt-Vertragstest ergänzt. Ein alter QML-Deklarationshinweis korrigiert.

- Native UI-Prüfung verwendet QtTest statt selbst konstruierter Pointerereignisse und sichert echte Renderbilder. Ein lokaler Entwicklungs-Einstieg (`--ui-source`) erlaubt die Sichtprüfung eigener QML-Dateien mit dem gebauten Backend; normale Starts verwenden unverändert die gebündelten Ressourcen.
- Qt-SDK und gepinnte Mac-Abhängigkeiten werden in CI wiederverwendet, damit UI-Korrekturen nicht jedes Mal dieselben Werkzeuge herunterladen/kompilieren.

## Übergabestand 28.10.2026

- Code `ece44ca` auf `codex/uflipper-foundation`; CI `40579017366` mit vier erfolgreichen Jobs. Keine Veröffentlichung.
- Fertiges Universal-Bundle auf Apple Silicon mit Standardressourcen gestartet; 17 reale QtTest-Klickschritte und 13 Renderbilder, Easy-Mode-Rundläufe in beiden Ansichten bestanden. Offline-Zustand sichtbar ohne erfundene Messwerte.
- `/int` bei exakt bekanntem Momentum-Stand korrekt als SD-gestützten Datenbereich bezeichnet. Bei unbekannter Firmware nur logische RPC-Dateisystemkapazität benennen, keine physische Flashgröße behaupten.
- Nächster fachlicher Schritt bleibt M2: UI, Live-Aufträge, MCP und Reader an einen gemeinsamen Verbindungsbesitzer anbinden. GUI-Live-Empfang, vollständiges Backup, Firmware-Bibliothek, alle Cloud-Adapter, MCP, weitere Layouts und Systemreader sind noch nicht fertig. IR-Wiederholung auf Nutzerwunsch vertagt; NFC-Kartennachweis wartet auf passende Testkarte.
