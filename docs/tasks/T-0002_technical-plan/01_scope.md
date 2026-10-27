# Umfang, Befund und offene Entscheidungen

## In dieser Runde geliefert

- Gegen einen festgehaltenen Momentum-/Fork-Stand geprüfte Funktionslandkarte für Dashboard, Dateien, IR, Sub-GHz, NFC, LF RFID, iButton/1-Wire, GPIO/Busse, USB/Bluetooth/BadKB, Apps und Pflege.
- Eigenständige Betrachtung des NFC-Systemreaders und der erlaubten SD-Hilfsprogramme.
- Vorschlag für gemeinsam genutzten Gerätebetrieb, Messdaten, Aufzeichnung und MCP; keine neue Stack-Auswahl.
- Entwürfe für Firmware-Bibliothek, Backup, Cloud-Sync, Arbeitsflächen und Easy Mode.
- Prüfschritte mit konkretem Ergebnisnachweis und Release-Gate.

## Gültigkeit der Aussagen

Quellprüfung: Momentum `mntm-012` auf `e1784e7418d8b074e971983ceb6fef0f37e52ae4`; Fork `1d26683ff5b751f219a4e40443b7c8355eb39469`. Dateihashes und Belege: [10_sources.md](10_sources.md), [sources.json](sources.json).

Das ist keine Aussage über den aktuellen Zustand eines angeschlossenen Flippers, andere Firmwarestände oder alle externen Apps. Niemand hat in dieser Runde Gerätefunktionen, Uploads, Cloud-Synchronisierung oder Systemreader ausprobiert. Eine technische Route ist erst nach einem beobachteten Ablauf freigabefähig.

## Festgelegt in der weiteren Auswahlrunde · 27.10.2026

| Bezug | Bestätigte Entscheidung |
| --- | --- |
| O01 · Cloud-Anbindungen | Google Drive, Dropbox, OneDrive, WebDAV und S3 gehören zum ersten öffentlichen Release (Produktentscheidung 19). |
| O02 · Backup-Fehler | Update/Wechsel/Restore anhalten, bis ein geprüftes Backup vorliegt; kein Bypass ohne Backup (18). |
| O05 · Verschlüsselungszugang | Wiederherstellungsschlüssel als Datei/Code und Masterpasswort als wählbare Verfahren anbieten; Kryptografie und Umstellung noch offen (21). |
| O06 · Löschen | Überall in den Papierkorb verschieben; endgültig erst manuell löschen. Automatische Bereinigung zunächst aus (20). |
| O09 · Startlayout | Feste Seitenleiste mit Arbeits- und Detailbereich beim ersten Start; alle drei Modi auswählbar. Visueller Entwurf noch abzustimmen (17). |
| O12 · Helfer-Updates | Jede neue Helferversion vor Installation ausdrücklich freigeben. Quellenvertrauen ersetzt diese Freigabe nicht (22). |

## Noch vor der jeweiligen Umsetzung zu klären oder nachzuweisen

Diese Punkte müssen nicht alle vor einem ersten, später genehmigten Prototyp beantwortet sein. Sie werden an der passenden Etappe geklärt.

| ID | Noch festzulegen oder nachzuweisen | Vorschlag oder Entscheidungspunkt |
| --- | --- | --- |
| O03 | Standard-Backup festgelegt | Alle lesbaren internen Dateien und gesamte SD inkl. Apps/Ressourcen. Konsistenz und Restore-Kompatibilität praktisch nachweisen. |
| O04 | Übernahme auf zweiten Computer festgelegt | Inhalte/Einstellungen synchronisieren; Cloud-Anmeldung, Entsperren und Pfadzuordnung auf jedem Computer separat. MCP lokal. |
| O05 | Verschlüsselungsformat und Schlüsselverwaltung | Beide gewählten Zugangswege umsetzen; sichere lokale Ablage, Schlüsselverlust, Umstellung und ggf. gemeinsame Verwendung konkretisieren. Noch kein Kryptoverfahren ausgewählt. |
| O06 | Papierkorb bei Offline-Nutzung und Konflikten | Löschmarkierungen/Revisionen, Wiederherstellung und manuelle endgültige Löschung über mehrere Computer technisch definieren. Gleichzeitige Bearbeitung und Löschung bleibt ein Konflikt mit Auswahl. Keine automatische Löschfrist vereinbart. |
| O07 | Systemreader-Plattform festgelegt | Erstes Release: Mac-Systemreader, Linux-NFC in der App. Unabhängiger PC/SC-Client: OpenSC; ISO-DEP-Testkarte. |
| O08 | Release-Plattformen festgelegt | macOS 13+ Universal Intel/Apple Silicon; Ubuntu 22.04+ x86_64, AppImage-Ziel. Mindestversionen tatsächlich prüfen. |
| O10 | Grenzen für Messdaten und seltene Geräte-Apps | Durchsatz, Speicherbedarf und App-/Modulabdeckung je Profil messen. Keine universelle Hochgeschwindigkeitsmessung oder Unterstützung jedes Drittanbieter-FAP behaupten. |
| O11 | Eigener App-Updatekanal und Veröffentlichung | Firmware-Bibliothek und uFlipper-App-Updates getrennt planen; Signierung/Packaging und Release-Abnahme vor Veröffentlichung. |

Kein offener Punkt ist Zustimmung zu einer empfohlenen Option. Die Etappen ordnen Arbeit; sie streichen keine bestätigten Funktionen aus dem Zielbild. Anpassungen des Kernumfangs vor dem ersten Release brauchen eine ausdrückliche Entscheidung.

Der verbindliche Umsetzungsauftrag einschließlich Hintergrundbetrieb ist in [T-0003](../T-0003_foundation/01_scope.md) dokumentiert. Die obigen technischen Nachweise sind Arbeitspakete, keine erneut offenen Produktentscheidungen.
