# Validierung · Stand 29.10.2026

**Geprüfter App-Stand: b010f9e. Kein vollständiges Gerätebackup nachgewiesen; Vollbackup-Start bewusst gesperrt, weil das ungefilterte Inventar fehlt (#13).**

## Pakete und Softwareverträge

- CI [40935865240](https://github.com/JoKeks2023/uFlipper/actions/runs/40935865240): alle 5 Jobs erfolgreich (Desktop Mac/Linux, Kern Mac/Linux, Reader-Komponente ohne Installation). Frühe Backup-/Leerdatei-Verträge, Protokollprüfungen und native Paketstarts erfolgreich.
- Kontrollierter Backupadapter: zweimaliges vollständiges Inventar und Inhaltsvergleich; 200.008 Byte/3 Dateien einschließlich leerer und verborgener Datei plus leerem Verzeichnis. Fehlende SD, Traversal, geändertes Inventar/Inhalt, überschrittener Datenblock, unbestätigter Transportfehler, Abbruch/späte Antwort, Wiederverbindung und Startwiederherstellung geprüft. Dies ersetzt keinen ungefilterten Geräteadapter.
- Unabhängiger Python-Prüfer: bestehende echte Testsicherung `ebffac75238c52629dd5db4501df4e71`, 2 Dateien/172 Byte korrekt; als Vollbackup zurückgewiesen. Dateibitfehler/Manifeständerung in separaten lokalen Kopien erkannt. Gerätedateien dabei unverändert.
- Mac-Artefakt 12924848854: Archiv `9bac48755299f40f77068e27b292530642ec2606a573356a53eb3cde5ca4608e`; Binary `68aa6a24e7518307638037d4f4a1a4ff769d6eb64fe30fb88eb6296eea466557`. Universal x86_64/arm64, Bundle-ID `com.jorisconrad.flipperzeroutil`, strenge Codesignprüfung erfolgreich.

## Tatsächlich am M1-Mac und Flipper beobachtet

- Native Paketbackend-Diagnose mit separater lokaler QML: `backendReady=true`, `nfcCanStart=true`, `fullBackupCanStart=false`; Inventarbegründung vorhanden. Kein Bildschirmseitenwechsel als Start-Workaround, kein NFC-Feld und kein Backup gestartet. Das ist ein Backend-Diagnosenachweis, kein vollständiger Backup-Produktablauf.
- Danach normale Produktoberfläche: 19 Klicks, 15 Screenshots, Easy-Mode-/Einstellungs-Dropdown-Rundlauf und 1060×680-Kompaktansicht; 0 erfasste QML-Fehler. Dashboard und Backup-Seite selbst visuell geprüft.
- Geprüftes Paket unter `build_mac/uFlipper.app` geöffnet; Vorversion unter `build_mac/previous-before-backup-b010f9e/` erhalten. [Sanitisierte Evidenz](evidence/mac-native-b010f9e.json). Lokale Originale: `build_mac/backup-gate-b010f9e/`.
- Linux-Oberfläche f66d0ee ebenfalls tatsächlich durchlaufen: 19 Klicks, 15 Screenshots, 0 erfasste QML-Fehler; kein angeschlossenes Linux-Gerät. [Ergebnis](evidence/linux-ui-f66d0ee.json). Der neue Sperrstand b010f9e hat dieselben CI-UI-Prüfungen bestanden.

## Gefundene und behobene Softwarefehler

- #9: erster Leerdatei-Protokolltest reproduzierte einen Nullzeiger-Absturz im vorhandenen StorageReadResponse. Datenlose Antworten werden nun sicher verarbeitet, verkürzte Transfers zurückgewiesen; frühe und vollständige Protokolltests grün.
- #10: verspäteter Queue-Rückruf durfte eine geschlossene RPC-Sitzung wieder als Idle melden. Geschlossene Sitzung, wiederholter Stop und späte Daten bleiben geschlossen; Stop leert die Queue und trennt den Port.
- #12: erster realer Startversuch mit f66d0ee fand eine Startblockade trotz erfolgreichem RPC-Handshake. Ready hing am optionalen Bildschirmstream, dessen vorzeitiger Start gesperrt war. B010f9e bindet Ready an den tatsächlichen aktuellen Gerätehandshake; oben genannter nativer Nachweis erfolgreich. Der erste Versuch begann kein Backup und gilt nicht als bestandener Abbruchtest.

## Offenes Vollständigkeitsgate

Die gepinnte Momentum-RPC-Dateiliste filtert nicht-ASCII-Namen. Zwei gleiche gefilterte Listen beweisen daher keine vollständige SD-Sicherung. Nicht behauptet wird, dass auf der konkreten SD tatsächlich solche Dateien fehlen. Ein ungefilterter, Ende/Lesefehler unterscheidender Inventaradapter bleibt erforderlich; SDK-Befund und Quellen unter [Umsetzung](04_impl_report.md). Kein neuer Helfer gebaut/installiert, keine Firmwareänderung.

Weiterhin offen: tatsächlicher Vollbackup-/Abbruch-/Fehlernachweis am Gerät, Produkt-Restore samt Kompatibilitätsvorschau und frische automatische Vollbackup-Vorkette. Der frühere abgegrenzte Restore-Test unter T-0003 ersetzt A15 nicht. Mac-Systemreader nicht installiert, vertagter IR-Test nicht gestartet. Kein Release und keine Kernabnahme.
