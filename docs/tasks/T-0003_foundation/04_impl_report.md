# Laufender Arbeitsbericht

27.10.2026: Branch angelegt; Planung auf 27 Entscheidungen ergänzt. Umsetzung läuft.

Implementiert: portable C++17-Kernmodule für gemeinsame Ressourcenbelegung, Konfliktentscheidung, Stop-Bestätigung und unbekannten Gerätezustand; SQLite-/Dateikatalog für fortlaufende Aufzeichnung, exklusiver Besitzer, Hashprüfung und Wiederauffinden nach Prozessabbruch; libsodium Secretstream-/Argon2id-Bausteine mit authentifizierter Veröffentlichung entschlüsselter Dateien.

Produktidentität und CI werden auf uFlipper ausgerichtet. Der Upstream-App-Updatekanal wird deaktiviert; Firmwarequellen bleiben getrennt. Neue CI erzeugt ausschließlich Testartefakte. Vollständige Cloud-Anbindungen, Schlüsselverwaltung, MCP und Systemreader sind damit noch nicht implementiert.

Befund: GitHub-Issues sind im Fork deaktiviert (`gh issue list`); der konkrete Updater-/CI-Bug konnte daher nicht als Issue erfasst werden. Die verbundene GitHub-Integration hat außerdem keine Schreibberechtigung für Issues. Keine Repo-Einstellungen eigenmächtig geändert.

Befund: bisheriger DMG-Dekorationspfad verweist auf einen nicht vorhandenen Background-Ordner. Der neue Testpaket-Ablauf verwendet eine einfache DMG mit Applications-Link.

## Nächster Zwischenstand

- Nativer serieller Empfangsprüfer mit demselben Belegungs-/Aufzeichnungskern ergänzt; begrenzte Lese-Puffer, exklusiver Port und bestätigter Ctrl+C-Stop.
- CI-Kernprüfung unter Ubuntu bestanden. Erster Workflow hatte einen YAML-Escapingfehler; korrigiert. Linux-Desktoplauf zeigte eine fehlende Suchwerkzeug-Abhängigkeit beim Setzen des Qt-Pfads; auf Python-Standardsuche geändert.
- macOS-Runner auf 15 umgestellt, da GitHub macOS 14 bis 02.11.2026 außer Betrieb nimmt; Deployment-Ziel bleibt macOS 13. Intel-Coreprüfung zusätzlich eingeplant. Quelle: https://github.com/actions/runner-images/issues/13518
- Drei visuelle Entwürfe vorbereitet und ausdrücklich als Beispieldaten gekennzeichnet; Produkt-UI noch nicht ersetzt.

## Empfang, App-RPC und geänderte Gestaltung

- Firmware-bestätigten Empfangsstart ergänzt. Momentum `subghz rx` braucht bei expliziter Frequenz zusätzlich den internen Geräteindex `0`; korrigiert. Früher CLI-Rücksprung beendet den Versuch als Fehler, statt einen erfolgreichen Empfang vorzutäuschen.
- Fehlerhafte/fehlende Aufnahme darf beim Wiederöffnen nicht den übrigen Katalog blockieren. Mit echtem Prozessabbruch und entferntem Sitzungsordner reproduziert und behoben; beschädigten Eintrag als unterbrochen ohne gültige Integritätsprüfung erhalten.
- App-RPC Start/Exit/DataExchange und Broadcast-Decodierung im bestehenden Qt-Gerätepfad ergänzt. Neue App-Operationen prüfen jede Fehlerantwort. Plugin-Interface-Version angehoben, damit alte Upstream-Plugins nicht mit verändertem Interface geladen werden.
- NFC-FAP 0.1.0 für das exakte Momentum-SDK gebaut. Quelle, Protokollgrenzen und Artifact-Hash unter `helpers/nfc_bridge/`; keine Installation oder Firmwareänderung.
- macOS-Desktop- und Core-CI auf verfügbaren Intel-Runner gelegt; Universal-Ziel bleibt Intel + Apple Silicon, lokaler ARM-Kerntest bestanden. ARM-GitHub-Runner warteten wiederholt; keine Plattform gestrichen.
- Neue Produktentscheidung 28: **gesamte App im qFlipper-Design**. Nutzer hat die drei hellen Entwürfe gesehen und abgelehnt. Neue Dashboard-/Werkzeugansichten nutzen das vorhandene Theme; größerer UI-Umbau weiterhin erst nach sichtbarer Prüfung.

## Dashboard-Daten und Wartungsgrenzen

- Power-Telemetrie über bestehendes Property-RPC, mit expliziten Einheiten, Rohwerten, Zeitstempel, fehlenden Werten und Veraltet-Status ergänzt; Formatmajor 2 wird ausgewertet. Lesen ist außerhalb des normalen Bereit-/Screen-Streaming-Zustands pausiert. App-Dashboard selbst noch nicht ersetzt.
- Separates natives `uFlipper-probe`: Power-Nachweis und Start/Info/Exit des bereits freigegebenen installierten Helfers; optional drei abhängige NDEF-Testkarten-APDUs mit vorher angelegter dauerhafter Aufnahme. Keine Installation/Flash-Funktion im Prüfprogramm.
- Befund: geerbte Wartungspfade haben noch keine Sicherung der vollständigen SD-Karte. Deshalb Firmwareänderungen und Restore in diesem Entwicklungsstand sichtbar blockiert, bis die geforderte Vollbackup-Prüfung tatsächlich verfügbar ist. Dies ist eine temporäre Entwicklungssperre, keine Reduktion des Release-Umfangs. CLI ohne Argumente zeigt Hilfe; startet kein Firmware-Update mehr.
- Nativen Linux-App-Start in CI ergänzt. Verifikation und Gerätetest des neuen Qt-Pfads stehen noch aus.

- Mac-Paketierung um explizite QML-Importübernahme ergänzt (`macdeployqt -qmldir` und lokale Importpfade); ohne diesen Schritt ist ein erfolgreicher Build kein eigenständig lauffähiges QML-Paket. Quelle: https://doc.qt.io/qt-6.8/macos-deployment.html. Nächster CI-Starttest verbirgt das Entwickler-Qt kurzzeitig und startet das gebündelte Programm, um fehlende Abhängigkeiten nicht zu übersehen.

## Backup-Dateiformat

- `BackupStore`: SQLite-Katalog, versioniertes gestreamtes Manifest und private Payload-Dateien. Dateien werden in begrenzten Chunks geschrieben; Gesamtinventar wird seitenweise gelesen. Leere Verzeichnisse bleiben erhalten, Traversal-Pfade werden abgewiesen.
- Vollgerät-Sicherungen und abgegrenzte Testsicherungen sind ausdrücklich verschiedene Scopes. Vollgerät-Abschluss verlangt bestätigte Inventare für intern und SD; neue Dateieinträge verwerfen die betroffene Inventarbestätigung. Nur nach erneutem Lesen und Hashprüfung wird lokal `verified` gesetzt.
- Dateiinhalt, Pfade, Größen, Scope und Metadaten sind an das kanonische Manifest gebunden. Unterbrochene/zu kurze Dateien, veränderte Katalogpfade und beschädigte Payloads ergeben keine gültige Sicherung.
- Shared SQLite-Hilfstypen aus dem Aufnahme-Modul herausgezogen; bestehende Aufnahmeprüfungen bleiben grün. Secretstream-Zustand und Klartextpuffer werden auch auf Fehlerpfaden aus dem Arbeitsspeicher gelöscht.
- Dies ist der lokale Speicherbaustein. Remote-Inventarvergleich, vollständiger Geräte-Download und selektiver Restore sind noch nicht angeschlossen; die Wartungssperre bleibt deshalb wirksam.
