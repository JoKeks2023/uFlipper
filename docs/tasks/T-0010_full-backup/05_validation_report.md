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

## Erster nativer Geräteversuch

CI-Stand f66d0ee: alle 5 Jobs erfolgreich. Mac-Artefakt 12923686938, SHA-256 `aceff4151ae2a982636c424834b808bf71c3d8cd8068e79e0a13c3855861c839`; Binary `2a1fb5531a38ac04f71a65e1c10dee540088525ff96e7a7ff3984a2a3af23463`, Universal x86_64/arm64, strenge Codesignprüfung und Bundle-ID korrekt.

Echter Flipper erkannt, RPC-Handshakes erfolgreich. Danach wurde die Backup-Schaltfläche nicht freigegeben: Backend-Ready hing bisher am optionalen Bildschirmstream, dessen vorzeitiger Start durch die neue Offline-Zugriffssperre verhindert wurde. Keine Sicherung begonnen; eigenen wartenden Test beendet. [#12](https://github.com/JoKeks2023/uFlipper/issues/12): Ready direkt an den aktuellen Gerätehandshake gebunden; neuer Paket-/Gerätetest erforderlich. CI-Erfolg war hier ausdrücklich kein Funktionsnachweis.

## Vollständigkeitsgate nach Quellenprüfung

Nicht behauptet: dass auf der konkreten SD tatsächlich nicht-ASCII-Dateien fehlen. Belegt: der vorhandene RPC-Inventarweg kann deren Abwesenheit nicht nachweisen. Native Vollbackup-Freigabe deshalb gesperrt, bis ein ungefilterter, Fehler/Ende unterscheidender Inventaradapter beobachtet ist. Kein vollständiger Gerätebackup-Erfolg; kein Release. Neue UI-/Protokoll-Gate-Prüfung folgt.
