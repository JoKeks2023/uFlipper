# Arbeitsgrenzen

Status: Diese Grenzen dokumentieren die abgeschlossene Planungsrunde. Der Nutzer hat anschließend den verbindlichen Umsetzungsplan ausdrücklich beauftragt; für die Umsetzung gelten [T-0003](../T-0003_foundation/00_brief.md) und die dort dokumentierten Freigabegrenzen.

- Aktueller Auftrag: planen, lesen, lokale Dokumentation ergänzen. Keine Änderungen am Anwendungscode, keine Builds, Geräteaktionen, Helferinstallationen, Betriebssystemtreiber, Cloud-Verbindungen, GitHub-Mutationen, Commits, Pushes oder Veröffentlichungen.
- Hilfsprogramme/Skripte auf SD sind nur nach ausdrücklicher Freigabe zulässig. Die beantwortete Produktfrage erlaubt das künftige Verfahren; sie genehmigt keine jetzige Installation. Momentum bleibt unverändert.
- Bestehende C++-/Qt-/QML-Basis untersuchen. Kein stiller Wechsel zu Web, SwiftUI, React Native oder anderem Stack. Die tatsächliche Anmutung und die Anforderungen an native Bedienelemente werden mit dem UI-Entwurf geprüft; Qt/QML bedeutet nicht automatisch, dass jedes Element ein Betriebssystem-Widget ist.
- macOS und Linux. Keine stillschweigende Reduktion auf Mac, IR/NFC oder Dateiverwaltung.
- Daten korrekt benennen: Pulszeiten, dekodierte Frames, nacheinander gemessener RSSI und Dateidumps sind unterschiedliche Datentypen. Keine erfundenen Pegel, I/Q-Daten oder Sampleraten.
- Vollzugriff über MCP bleibt computergebunden. Ein Synchronisationslauf erteilt auf einem anderen Computer keine Freigabe.
- M1/8 GB: keine Simulatoren/Emulatoren; spätere schwere Builds möglichst über CI.
- Feste UI-Werte als Auswahl/Dropdown; echte Sonderfälle über „Andere…“.
- GPLv3 des Forks erhalten. Frühere MIT-/Release-Entscheidungen des eigenständigen Prototyps nicht auf qFlipper übertragen.
- Kein Squash-Merging. Diese Planungsrunde erzeugt ohnehin weder Commit noch PR.
- Codebefunde sind Planungshinweise. Die ausdrücklich vereinbarte Grenze „keine GitHub-Änderungen“ verhindert das automatische Anlegen von Issues in dieser Runde.

## Ergänzte verbindliche Produktregeln · 27.10.2026

- Update, Firmwarewechsel und Restore bleiben bei fehlgeschlagenem Backup angehalten, bis ein geprüftes Backup vorliegt; derselbe Ablauf gilt über UI und MCP.
- Jede neue Helferversion braucht vor Installation eine ausdrückliche Freigabe. Eine Quellenfreigabe erlaubt keine automatischen Versionsupdates.
- Cloud-Löschungen führen überall in den Papierkorb; endgültige Löschung nur manuell. Keine automatische Löschfrist eingeführt.
- Schlüsseldatei/-code und Masterpasswort sind wählbare Zugangswege. Die Auswahl entscheidet noch keinen kryptografischen Algorithmus und erteilt keine Cloud-Einrichtungsfreigabe.
