# Bestätigter Umfang und Entscheidungen

## Arbeitsbereiche

| Bereich | Bestätigtes Ziel |
| --- | --- |
| Werkzeuge | Alle Flipper-Gerätebereiche berücksichtigen; geeignete Desktop-Abläufe für Erfassen, Live-Beobachtung, Einstellungen, Ausführung und Auswertung. |
| Bibliothek | Aufnahmen, Signale, Karten, Skripte und andere unterstützte Dateien organisieren, suchen, bearbeiten und vergleichen. |
| Live-Labor | Live-Daten am Computer darstellen und untersuchen; Sitzungen aufzeichnen und später auswerten. IR und NFC sind Beispiele, keine Begrenzung des Umfangs. |
| NFC-Systemintegration | Karten in uFlipper lesen und auswerten; zusätzlich Nutzung als Lesegerät durch kompatible andere Mac-Programme ermöglichen. Anbindung und Grenzen noch prüfen. |
| Firmware | Firmware-Angebote in der App finden, herunterladen, installieren und aktualisieren; Firmwarewechsel und Versionsinformationen berücksichtigen. |
| Backups | Gerätesicherungen erstellen, verwalten, prüfen und wiederherstellen. Der tatsächlich sicherbare und wiederherstellbare Inhalt ist noch zu bestimmen. |
| Dashboard / Gerätepflege | Verbindung, Geräte-, Akku-, Power-, Firmware- und Speicherinformationen sowie laufende Vorgänge sichtbar machen; bestehende qFlipper-Pflegefunktionen berücksichtigen. |
| MCP | Dieselben Gerätezustände und Funktionen über das gemeinsame Backend zugänglich machen. |

Die Funktionslandkarte muss unter anderem Sub-GHz, IR, NFC, 125-kHz-RFID, iButton, GPIO/serielle Schnittstellen, USB-/Bluetooth-Funktionen, Geräte-Apps und Skripte berücksichtigen. Welche konkreten Aktionen und Daten je Bereich möglich sind, bleibt Gegenstand der Prüfung. Keine pauschale Annahme gleichzeitiger Nutzung oder identischer Fähigkeiten aller Bereiche.

## 27 ausdrücklich gewählte Produkteigenschaften

| Nr. | Thema | Nutzerentscheidung |
| --- | --- | --- |
| 1 | Firmware-Angebote | Momentum und offizielle Firmware; zusätzlich frei erweiterbare Quellen. |
| 2 | Automatische Backups | Vor Firmware-Updates, Firmwarewechseln und Wiederherstellungen; manuelle Backups ebenfalls möglich. |
| 3 | Ablage | Lokale Ablage plus integrierte Cloud-Synchronisierung. |
| 4 | Cloud-Anbindung | Bestehende Cloud-Anbieter und eigener Speicher. Ein eigener uFlipper-Cloud-Dienst wurde nicht beauftragt. |
| 5 | Synchronisationsumfang | Alles einschließlich Einstellungen automatisch synchronisieren, sobald die entsprechende Cloud-Anbindung eingerichtet ist. |
| 6 | Verschlüsselung | Verschlüsselung vor dem Upload ist eine wählbare Option. |
| 7 | Gerätekonflikt während Messung | Vor einer anderen Aktion nachfragen, ob die laufende Messung beendet werden soll. Keine stillschweigende Unterbrechung. |
| 8 | Live-Aufzeichnungen | Jede gestartete Live-Sitzung automatisch dauerhaft speichern. |
| 9 | MCP | Voller Zugriff nach bewusster Gesamtfreigabe auf dem jeweiligen Computer. Die Freigabe ist computergebunden. |
| 10 | Layout | Drei vom Nutzer wählbare Modi: flexible Workbench mit Tabs/andockbaren Bereichen; feste Seitenleiste mit Arbeits-/Detailbereich; unabhängige Werkzeugfenster für mehrere Bildschirme. |
| 11 | Easy Mode | Geführte Aufgaben innerhalb derselben Werkzeuge; erweiterte Parameter bei Bedarf einblenden. |
| 12 | Veröffentlichung | Erst nach Abnahme des vereinbarten Kernumfangs öffentlich veröffentlichen. |
| 13 | Synchronisationskonflikte | Nur den betroffenen Eintrag anhalten und eine Auswahl verlangen. Keine automatische Auswahl der neuesten Version. |
| 14 | Verschlüsselungsstandard | Bei jeder neu eingerichteten Cloud-Verbindung ausdrücklich zwischen verschlüsselt/unverschlüsselt wählen lassen; kein stillschweigender Standard. |
| 15 | Aufbewahrung | Grenzen für Alter, Anzahl oder Speicherverbrauch wählbar; automatische Bereinigung zunächst aus. |
| 16 | Hilfsprogramme auf SD | Eigene Hilfsprogramme und Skripte nach ausdrücklicher Freigabe erlaubt; Momentum selbst bleibt unverändert. Keine Installationsfreigabe für die aktuelle Planungsrunde. |
| 17 | Startansicht | Beim ersten Start feste Seitenleiste mit Arbeits- und Detailbereich. Alle drei Layoutmodi bleiben jederzeit auswählbar. |
| 18 | Backup-Fehler | Firmware-Update, Firmwarewechsel oder Wiederherstellung anhalten, bis ein geprüftes Backup vorliegt. Kein Fortfahren ohne geprüftes Backup. |
| 19 | Cloud im ersten Release | Google Drive, Dropbox, OneDrive sowie eigener Speicher über WebDAV und S3 gehören bereits zum ersten öffentlichen Release. |
| 20 | Synchronisierte Löschungen | Auf allen Computern und in der Cloud in einen Papierkorb verschieben; endgültig erst manuell löschen. Automatische Bereinigung bleibt zunächst aus. |
| 21 | Zugang zu verschlüsselten Daten | Zwei wählbare Wege: Wiederherstellungsschlüssel als Datei/Code oder selbst gewähltes Masterpasswort. Beide Verfahren anbieten; keine Pflicht, beide gemeinsam zu verwenden. Kryptografisches Format und Wechselverfahren noch festzulegen. |
| 22 | Helfer-Updates | Jede neue Version eines uFlipper-Hilfsprogramms vor der Installation ausdrücklich freigeben. Keine automatischen Helfer-Updates allein aufgrund einer zuvor freigegebenen Quelle. |
| 23 | Standard-Backup | Alle über die Geräteschnittstelle lesbaren internen Dateien und die gesamte SD-Karte einschließlich Apps und Ressourcen. |
| 24 | Zweiter Computer | Inhalte und Einstellungen synchronisieren; Cloud-Anmeldung, Entsperren und lokale Pfadzuordnung auf jedem Computer separat. MCP-Freigaben bleiben lokal. |
| 25 | Release-Plattformen | macOS ab 13 für Intel und Apple Silicon als Universal-App; Linux ab Ubuntu 22.04 x86_64 als AppImage. |
| 26 | Systemreader | Im ersten Release für andere Mac-Programme; Linux erhält die NFC-Werkzeuge innerhalb von uFlipper. |
| 27 | Hintergrundbetrieb | Optional mit sichtbarem Menüleisten-/Tray-Status; ohne Tray bleibt ein Statusfenster. Vollständiges Beenden stoppt Geräteaufgaben und sichert Aufnahmen. |

| 28 | Verbindliche Designsprache (27.10.2026) | Die gesamte App soll im Design von qFlipper bleiben. Die hellen Entwürfe wurden abgelehnt; vorhandenes Theme und Komponenten weiterverwenden. Vor größerem Umbau erneut sichtbar prüfen. |

## Weitere bestehende Vorgaben

- macOS und Linux; qFlipper als bestehende C++-/Qt-/QML-Basis.
- Erste Version mit unveränderter Momentum-Firmware. Freigegebene Hilfsprogramme/Skripte auf SD sind zulässig; Änderungen der Firmware selbst sind nicht vereinbart.
- Gestaltung: kompakt, sachlich und dicht. Die drei Layoutmodi verändern die Anordnung, nicht diese Gestaltungsrichtung.
- Zuletzt gewünschte macOS-Bundle-ID: `com.jorisconrad.flipperzeroutil`. Im Umsetzungsbranch gesetzt; Paket-/Laufzeitprüfung steht noch aus.
- Der qFlipper-Fork unterliegt der vorhandenen GPLv3-Lizenz; die MIT-Auswahl für den früheren, eigenständigen Prototyp ersetzt diese Lizenz nicht.

## Noch nicht entschieden oder nachgewiesen

- Konkrete Aktionen und Live-Daten je Gerätebereich, Firmware-/Protokollversion und Betriebssystem.
- Technische Anbindungen, Berechtigungen und konkrete Server-/Kontokompatibilität der gewählten Release-Anbindungen Google Drive, Dropbox, OneDrive, WebDAV und S3.
- Integrationsformat für frei ergänzbare Firmware-Quellen, Paketkompatibilität und Update-/Fehlerverhalten.
- Exakter Backup-Inhalt, Konsistenz während einer Sicherung und Wiederherstellung über Firmware-/Versionsgrenzen hinweg.
- Technischer Weg und Kompatibilitätsumfang der NFC-Systemintegration; das Mac-Ziel ist ausdrücklich verlangt, ein entsprechendes Linux-Verhalten noch abzugrenzen.
- Speicher- und Synchronisationsmodell für große Live-Aufnahmen, Messzeitpunkte, Lücken und unterbrochene Sitzungen.
- Verteilung übertragbarer Einstellungen gegenüber lokalen Pfaden, Zugangsdaten und computergebundenen MCP-Freigaben. Die Gesamtfreigabe für einen Computer darf nicht als Freigabe eines anderen Computers interpretiert werden.
- Schlüsselverwaltung, kryptografisches Format und Folgen eines Wechsels zwischen Wiederherstellungsschlüssel und Masterpasswort; beide Zugangswege sind als Auswahl bestätigt, die technische Architektur bleibt offen.
- Offline-Löschkonflikte, Wiederherstellung aus dem gemeinsamen Papierkorb und technische Umsetzung der endgültigen manuellen Löschung; die Papierkorb-Regel selbst ist bestätigt.
- Visuelle Gestaltung und konkrete Bildschirmabläufe, unterstützte Systemversionen, Build-/Signierungsweg und Release-Abnahmeumfang. Die feste Seitenleiste ist als erster Layout-Standard bestätigt.

Diese Punkte sind offene Planung/Prüfung, keine automatisch zusätzlich vereinbarten Produktfunktionen.

Quellprüfung und technische Vorschläge vom 27.10.2026: [T-0002](../T-0002_technical-plan/00_brief.md). Dort sind erste Wege eingeordnet; Laufzeitnachweise und gemeinsame Entscheidungen stehen weiter aus.
