# Abnahme der Planung und weitere Prüfschritte

## Stand dieser Planungsrunde

- [x] Das übergreifende Ziel und alle 27 Multiple-Choice-Entscheidungen (ergänzt am 27.10.2026) dokumentiert.
- [x] Bestätigte Anforderungen von offenen technischen Fragen getrennt.
- [x] Keine Implementierung oder Veröffentlichung aus den Produktauswahlen abgeleitet.
- [x] Quellenbezogene Funktionsmatrix für alle Gerätebereiche erstellt und gegen festgelegtes Momentum mntm-012 geprüft: [T-0002](../T-0002_technical-plan/06_function_matrix.md). Keine Laufzeitprüfung am Gerät.
- [ ] Die erreichbaren Funktionen und verbleibenden Lücken gemeinsam bewertet.
- [ ] Oberflächenentwürfe für die drei Layoutmodi und Easy Mode durchgegangen.
- [ ] Konkrete Umsetzungsetappen und Release-Abnahmekriterien abgestimmt.

## Nächster fachlicher Nachweis: Funktionsmatrix

Für jede konkrete Aktion erfassen:

1. Gerätebereich und gewünschter Arbeitsablauf.
2. Verfügbare Schnittstelle und genaue Firmware-/Protokollversion mit Quellenbeleg.
3. Status: im Code belegt, am Gerät geprüft, noch offen oder unter den Randbedingungen nicht erreichbar.
4. Daten und Grenzen: Format, Einheiten, Zeitbezug, Aktualisierung, mögliche Messlücken.
5. Gerätebelegung, Start/Stop, Fehler-/Abbruchverhalten und mögliche Parallelität.
6. Passender Desktop-Ablauf sowie Nutzung über MCP oder andere Programme, soweit vorgesehen.
7. Überprüfbares Abnahmeszenario auf macOS und Linux; Abweichungen sichtbar ausweisen.

Quellprüfung ist kein Funktionsnachweis am Gerät. Ein grüner Build ist kein Nachweis einer Gerätemessung, Cloud-Synchronisierung, Wiederherstellung oder NFC-Systemintegration.

## Danach zu konkretisieren

- Firmware-Verwaltung: Quellen, Versionserkennung, Paketwahl, Download, Backup, Installation und Fehlerfall.
- Backups: Dateninventar, Integritätsprüfung, Wiederherstellung und Versionsgrenzen.
- Cloud: konkrete Anbindungen, Synchronisation einschließlich Einstellungen, Konflikte, explizite Verschlüsselungswahl, Aufbewahrung.
- Oberfläche: feste Grundstruktur, flexible Workbench und separate Fenster; dieselben Funktionen und Zustände in allen Modi.
- Eine nachvollziehbare Abnahmeliste für den ersten öffentlichen Release. Noch keine Versionsnummer oder Termin zugesagt.
