# Validierung

## Bisher beobachtet

- Lokaler kleiner C++-Build unter macOS/arm64: `ctest` 3/3 erfolgreich (Speicher/Krypto/Aufträge, serieller Ablauf, NDEF). Kein vollständiger lokaler Qt-Build.
- CI [40929099330](https://github.com/JoKeks2023/uFlipper/actions/runs/40929099330), Quellstand `807e62e`: Kernprüfungen Mac/Linux erfolgreich; frühe Linux-Backupverträge erfolgreich. Erster vollständiger Linux-App-Build erfolgreich; der neue Leerdatei-Protokolltest reproduzierte anschließend einen Nullzeiger-Absturz im vorhandenen StorageReadResponse. Datenlose Antworten werden nun sicher verarbeitet, verkürzte Transfers zurückgewiesen; erneute Gesamtprüfung erforderlich.
- Backupverträge: vollständige zwei Inventare, zweimaliger Inhaltsvergleich, gestreamte 200.008 Byte, drei Dateien einschließlich leerer Datei, verborgene Datei, leeres Verzeichnis; fehlende SD, Traversal-Pfad, geändertes Inventar/Inhalt, zu großer Datenblock, unbestätigter Transportfehler, Abbruch/späte Antwort und neue Verbindung. Transport wird dabei kontrolliert ersetzt; kein Gerätenachweis.
- Unabhängiger Python-Prüfer an vorhandener echter `test-subset`-Sicherung `ebffac75238c52629dd5db4501df4e71`: 2 Dateien/172 Byte korrekt; als Vollbackup zurückgewiesen. In separater lokaler Kopie Dateibitfehler und Manifeständerung erkannt. Keine Gerätedateien verändert.

## Noch offen

Native UI und vollständige Gerätesicherung am Flipper tatsächlich ausführen; Abschlussdaten unabhängig prüfen; Mac-/Linux-Paketierung und UI-Prüfungen abschließen. Der vorherige abgegrenzte Restore-Test unter T-0003 ersetzt diesen Nachweis nicht. Kein Release, keine Kernabnahme.

## Korrigierter Desktop-Stand f66d0ee

CI [40931426982](https://github.com/JoKeks2023/uFlipper/actions/runs/40931426982): vollständiger Linux-Build, frühe Backup-/Leerdatei-Verträge, App-RPC-Prüfungen und nativer Start erfolgreich. UI tatsächlich durchlaufen: 19 Klicks, 15 Screenshots, 0 erfasste QML-Fehler; Backup-Seite visuell angesehen. Kein angeschlossenes Linux-Gerät. [Ergebnis](evidence/linux-ui-f66d0ee.json). Mac-Universal-Paket noch in Prüfung.
