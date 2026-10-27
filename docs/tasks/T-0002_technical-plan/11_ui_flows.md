# Oberfläche und Bedienabläufe · Entwurf zur Besprechung

Gestaltungsrichtung bestätigt: kompakt, sachlich und dicht. Ziel ist eine benutzbare Desktop-Werkzeugfläche auf macOS und Linux. Dieser Ablaufentwurf ist noch kein visueller Prototyp und keine Design-Abnahme. Kein QML geändert.

## Gemeinsame Grundstruktur

Vorgeschlagene Navigation: **Übersicht · Werkzeuge · Bibliothek · Firmware · Backups**. Einstellungen über einen festen Eintrag. Werkzeuge nach Hardwarebereich gruppieren; laufende Aufgaben und Verbindungszustand jederzeit erreichbar. Eine klare Hauptaktion je Werkzeug und sichtbarer Stop während aktiver Gerätefunktion.

Wenige Akzentfarben: aktive Handlung, Warnung/Fehler und echter Verbindungsstatus. Daten als ausgerichtete Zeilen/Tabellen, klare Typografie, kleine konsistente Abstände; keine Wand aus gleich gewichteten Karten. Technikdetails in Details/Erweitert, wenn sie eine Entscheidung erklären. Keine Produktions-UI mit Debug-Klassennamen.

## Startseite als Dashboard

Vorschlag für die feste Ansicht:

```text
uFlipper        Gerät auswählen       Verbunden · Momentum …       Aufgaben 1
────────────────────────────────────────────────────────────────────────────
Übersicht       Gerät und Firmware                  Akku / Versorgung
Werkzeuge       Name, Modell, Revision, RPC          %, lädt, Strom, Spannung
Bibliothek      SD, freier/belegter Speicher         Temperatur, weitere Werte*
Firmware        ────────────────────────────────────────────────────────────
Backups         Aktive Sitzung · Sub-GHz             Letzte Sicherung
                Parameter · Datenrate* · Dauer       geprüft / unvollständig
                Aufnahme läuft       [Stop]          [Backup erstellen]
                ────────────────────────────────────────────────────────────
                Zuletzt verwendet · Aufnahmen · Synchronisationsstatus
```

`*` Nur Werte anzeigen, die tatsächlich geliefert bzw. gemessen werden, mit korrekter Einheit. Werte mit Alter/Quelle und bei Bedarf Rohschlüssel im Detailbereich. Offline/fehlender Akkuwert wird nicht „0 %“. Keine zufälligen Beispieldaten als reale Gerätedaten zeigen. Die endgültige Auswahl der Powerfelder ergibt sich aus D01/D02.

Schnellzugriffe führen in das passende Werkzeug oder starten den üblichen Backup-Ablauf. Eine Gerätekachel öffnet Details; sie löst keine Emulation, Installation oder Rücksetzung aus.

## Ein Werkzeug, drei Anordnungen

| Wählbarer Modus | Anordnung | Bleibt gleich |
| --- | --- | --- |
| Feste Arbeitsfläche | Bestätigte Startansicht: Seitenleiste mit Arbeits- und Detailbereich. Später frei zwischen allen drei Layouts wechseln. | Fähigkeiten, Daten, Status, Easy Mode und Auftrags-IDs. |
| Flexible Workbench | Tabs, andockbare Ansichten für Signal, Tabelle, Parameter, Bibliothek; gespeichertes Layout. | Dieselbe Gerätesitzung; zusätzliche Ansicht startet keine zweite Messung. |
| Werkzeugfenster | Unabhängige Fenster pro Werkzeug/Ansicht, auf mehrere Bildschirme verteilbar. | Belegung zentral sichtbar; Schließen einer Ansicht beendet keine Messung stillschweigend. |

Im Fenster-Schließfall muss eine aktive Sitzung erhalten sichtbar erreichbar bleiben oder eine explizite Stop-Entscheidung bekommen; konkrete Regel vor Umsetzung festlegen. Keine versteckt weiterlaufende Gerätefunktion. Layoutwechsel verändert die Darstellung, nicht laufende Parameter oder Messdaten. Easy Mode ist keine vierte, abgeschnittene App.

## Easy Mode im selben Werkzeug

Beispiel „IR-Signal aufnehmen“: Ziel wählen → Gerät/Fähigkeit prüfen → „Aufnahme starten“ → Live-Ergebnis mit verständlicher Rückmeldung → benennen/zuordnen/erneut auswerten. Die Aufnahme wird bereits beim Start automatisch gespeichert; Benennen ist kein versteckter zusätzlicher Speicherschritt.

„Erweitert“ zeigt dieselben wirksamen Parameter, zusätzliche Analyse und Rohdaten. Bekannte Werte wie Protokoll, Preset, Frequenzband, Pinmodus, Baudrate oder Speicherprofil sind Auswahllisten. Echte Sonderwerte über „Andere…“ mit passender Validierung. Ein Beispiel darf nicht ungeprüfte Steuerwege als möglich darstellen.

Übergreifende Abläufe:

| Situation | Vorgeschlagener sichtbarer Ablauf |
| --- | --- |
| Live-Werkzeug | Parameter → Fähigkeit/Belegung prüfen → lokales Aufnahmeziel prüfen → starten → Ansicht + gespeicherte Daten + Stop. Kein manueller Speicherklick nötig. |
| Hardware belegt | Aktive Messung benennen und Folgen nennen → „Messung beenden und fortfahren“ / „Abbrechen“. Bis zur Entscheidung läuft sie weiter. |
| Helfer fehlt / neue Version | Zweck, Quelle/Version, SD-Pfad und benötigte Funktionen erklären → ausdrückliche Freigabe oder Abbrechen. Jede neue Version vor Installation freigeben; keine stillen Updates. |
| NFC-Systemreader | „Für andere Programme bereitstellen“ als eigener Betriebsmodus mit Karten-/Slotstatus und Stop. Keine Gleichsetzung mit „Karte lesen“. |
| Firmwarewechsel | Quelle/Version wählen → Ziel/Verträglichkeit prüfen → Backup → installieren → Verbindung/Version prüfen. Bei Backupfehler anhalten, bis ein geprüftes Backup vorliegt. |
| Restore | Sicherung/Inhalte wählen → Kompatibilität/Änderungen prüfen → aktuellen Stand sichern → wiederherstellen → Ergebnis pro Bereich prüfen. Ohne geprüftes Sicherheitsbackup bleibt Restore angehalten. |
| Sync-Konflikt | Betroffenen Eintrag und Basis/Beide Fassungen zeigen; Auswahl verlangen, übrigen Sync weiterführen. Lösungsoptionen vor Implementierung konkretisieren. |

## Einstellungen und Credits

Vorgeschlagene Gruppen:

- **Allgemein:** Sprache, Erscheinung, Layoutmodus, Easy Mode, Startverhalten. Erstes Layout festgelegt: Seitenleiste mit Arbeits- und Detailbereich. Weitere Defaults und konkrete Sprachen noch festlegen.
- **Gerät und Werkzeuge:** Verbindung, Gerätestandard, Helferstände und Status; keine globale Option, die alle Hardwarefunktionen automatisch startet.
- **Speicher und Aufzeichnung:** lokale Ablage, Platzbedarf, Auto-Aufzeichnung erläutern, Aufbewahrungsgrenzen und zunächst ausgeschaltete Bereinigung.
- **Cloud:** Auswahl von Google Drive, Dropbox, OneDrive, WebDAV oder S3; Verbindungen, Sync-/Konfliktstatus und Verschlüsselungswahl/-zustand. Bei Verschlüsselung wählbarer Zugang per Schlüsseldatei/-code oder Masterpasswort. Papierkorb mit Wiederherstellen und gesonderter manueller endgültiger Löschung; keine automatische Leerung.
- **Integrationen:** NFC-Reader und MCP mit computergebundener Gesamtfreigabe, aktiven Verbindungen und Widerruf.
- **Über uFlipper:** Version, Quellen/Projektlink, GPLv3, qFlipper-/Momentum-/Abhängigkeits-Credits und überprüfte Kompatibilität. Keine unzutreffende MIT-Lizenz aus dem Prototyp übernehmen.

Vor der UI-Implementierung ein echtes visuelles Beispiel für Dashboard, ein aktives Werkzeug, leeren/offline Zustand und Einstellungen zeigen. Diese gemeinsame Gestaltungskontrolle ist ein vorgeschlagenes Arbeits-Gate aus dem Transparenzwunsch, keine bereits erfolgte Zustimmung zum Entwurf.
