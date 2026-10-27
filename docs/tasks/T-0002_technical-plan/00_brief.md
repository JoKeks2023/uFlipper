# T-0002 · uFlipper: technischer Plan zur gemeinsamen Prüfung

Stand: 27.10.2026 · **Planungsentwurf, keine Implementierungs- oder Installationsfreigabe.**

uFlipper wird als professionelles Kontrollzentrum für macOS und Linux geplant. Der Flipper stellt die Hardware, der Computer die größere Arbeitsfläche, Auswertung und Organisation. Alle Gerätebereiche gehören zum Zielbild. Basis bleibt der qFlipper-Fork; Momentum bleibt zunächst unverändert. Eigene Hilfsprogramme und Skripte auf der SD-Karte sind nach ausdrücklicher Freigabe erlaubt. Diese Produktentscheidung erlaubt heute keine Installation.

Die bestätigten Entscheidungen stehen in [T-0001](../T-0001_product-plan/01_scope.md). Dieses Paket übersetzt sie in prüfbare Wege, Grenzen und Arbeitsetappen. Vorschläge für interne Abläufe sind als solche gekennzeichnet; offene Entscheidungen werden nicht durch die Dokumentation entschieden.

## Was die Recherche bereits klärt

1. **Viele Gerätewege sind vorhanden, der Desktop-Ausbau fehlt.** Momentum bietet je nach Bereich CLI, RPC und Scriptmodule. Der Fork enthält Datei-, Geräte-, Update- und Bildschirmbausteine, aber noch keine gemeinsame Werkzeug-/Streaming-Schicht. „Im Code vorhanden“ bedeutet nicht „in uFlipper fertig“.
2. **Live-Daten sind je Bereich verschieden.** IR liefert dekodierte Signale/Pulszeiten; Sub-GHz liefert Pakete/Pulszeiten, ein RSSI-Sweep wäre ein Helfer-Kandidat. Ein optisches IR-Spektrum oder allgemeine SDR-I/Q-Daten darf die Oberfläche daraus nicht machen.
3. **Der NFC-Systemreader ist das größte Integrationsrisiko.** Die vorhandene APDU-CLI verarbeitet eine vorab übergebene Liste. Dauerhafte Kartensitzung, Reader-Slot im Betriebssystem und eine echte andere Mac-Anwendung sind eigene Nachweise. Der CCID-Debugtest ist kein fertiger NFC-Reader.
4. **Der Backupper braucht mehr als den alten Backupknopf.** Der geprüfte qFlipper-Pfad sichert `/int`. SD-Inhalte, konsistenter Datenstand, Integritätsprüfung und Wiederherstellung müssen als eigener Ablauf geplant und getestet werden.

Belege und Grenzen: [Funktionsmatrix](06_function_matrix.md), [Quellenregister](10_sources.md). Kein aktueller Geräte- oder Betriebssystemnachweis liegt vor.

## Lesereihenfolge

| Dokument | Zweck |
| --- | --- |
| [01 · Umfang](01_scope.md) | Was dieses Paket leistet; noch zu treffende Entscheidungen. |
| [06 · Funktionsmatrix](06_function_matrix.md) | 65 konkrete Funktionen/Ziele, Datenprofile und Quellen. [CSV](function_matrix.csv) zum Filtern. |
| [07 · Gemeinsamer Betrieb](07_runtime_architecture.md) | Vorschlag für Verbindung, Hardwarebelegung, Helfer, Aufnahme und MCP. |
| [08 · Firmware, Backup und Cloud](08_firmware_backup_cloud.md) | Abläufe, Dateninventar, Konflikte und offene Entscheidungen. |
| [11 · Oberfläche](11_ui_flows.md) | Dashboard, Werkzeuge, drei Layoutmodi und Easy Mode als Ablaufentwürfe. |
| [09 · Etappen und Abnahme](09_milestones_acceptance.md) | Reihenfolge, Nachweisschritte und Kriterien für die öffentliche Veröffentlichung. |
| [02 · Planungsabnahme](02_done_when.md) | Erstellte Unterlagen gegenüber noch offener gemeinsamer Abnahme. |
| [03 · Grenzen](03_constraints.md) | Autorisierter Arbeitsrahmen. |
| [04 · Arbeitsbericht](04_impl_report.md) / [05 · Prüfbericht](05_validation_report.md) | Was in dieser Runde tatsächlich gemacht und geprüft wurde. |

## Vorschlag für den nächsten gemeinsam freizugebenden Schritt

Nach Prüfung dieses Plans eine begrenzte Machbarkeitsrunde: Verbindungsbesitz und Stop, automatisch gespeicherter IR-/Sub-GHz-Empfang, ein ausdrücklich genehmigter Helfer und ein dauerhaft vom Mac erreichbarer NFC-Kartenkanal. Backup-/Restore-Test mit klar abgegrenzten Testdaten ergänzen. Erst mit diesen Ergebnissen die produktive Umsetzung im Detail festlegen. Keine Freigabe wird aus dem Lesen dieses Dokuments abgeleitet.

## Weitere Auswahlrunde · 27.10.2026

Entscheidungen 17–22 sind ausdrücklich bestätigt: feste Seitenleiste als Startansicht; Update/Wechsel/Restore bis zu einem geprüften Backup anhalten; Google Drive, Dropbox, OneDrive, WebDAV und S3 bereits im ersten Release; Löschungen überall in den Papierkorb und endgültig nur manuell; Wiederherstellungsschlüssel und Masterpasswort als wählbare Zugangswege für verschlüsselte Daten; jede neue Helferversion erneut ausdrücklich freigeben. Alle drei Layouts bleiben auswählbar, automatische Bereinigung bleibt zunächst aus.

Dies konkretisiert Produktverhalten. Es startet keine Implementierung, Cloud-Verbindung, Installation oder Geräteaktion.
