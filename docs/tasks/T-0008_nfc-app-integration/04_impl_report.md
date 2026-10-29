# Umsetzung

- GitHub-Arbeitsticket: [#3](https://github.com/JoKeks2023/uFlipper/issues/3), In Progress. Huly ist in dieser Sitzung nicht verbunden.
- `NfcSession` verwaltet Start/ACK/Broadcast/Antwort/Stop, Zeitgrenzen und lokale Originalaufnahmen über CaptureStore/OperationCoordinator. Start ohne beschreibbare Ablage wird abgewiesen.
- `NfcController` verwendet ausschließlich DeviceRegistry/ProtobufSession der App. Prüft Firmwareziel, Größe und SHA-256 des bereits freigegebenen Helfers; keine Installation.
- NFC-Seite im vorhandenen qFlipper-Raster: Start/Stop, Kartenstatus, feste PPSE-Auswahl, Antwortdaten und gespeicherte Sitzungen.
- Konkurrierende Bildschirm-/Datei-/Firmwareansichten fragen vor dem Stop nach und warten auf Bestätigung. Backend-Sperren verhindern parallele Eingaben und Dateiaktionen auch unabhängig von der Navigation.
- Native Prüfoptionen `--review-nfc DIR` und optional `--review-nfc-card` klicken die echte Oberfläche. Kein automatischer Gerätetest in CI und kein Simulator.

Dateizugriffe werden anhand tatsächlich laufender Operationen gesperrt; die bisherige 500-ms-Verzögerung der Fortschrittsanzeige wird nicht als Gerätesperre verwendet. Die NFC-Sitzung bleibt in Verbindungsleiste und Dashboard sichtbar. Beim Wiederöffnen bleiben die ursprünglichen monotonen Antwortzeitpunkte erhalten.

Beim ersten nativen Gerätelauf wurden zwei Fehler gefunden und in `be2a115` korrigiert: unveränderte RPC-Verbindungen lösten wiederholt Telemetrie aus ([#4](https://github.com/JoKeks2023/uFlipper/issues/4)); die Helfer-Vorprüfung verglich das normalisierte Geräteziel `f7` mit `7` ([#5](https://github.com/JoKeks2023/uFlipper/issues/5)). Beide Fälle haben Regressionstests.

Der begrenzte PPSE-Arbeitsablauf ist implementiert und am angeschlossenen Flipper aus der nativen Mac-App geprüft: Start, Stop, Navigation mit Freigabedialog, zwei Apple-Pay-Antworten, automatische Aufnahme und Wiederöffnen. Mac-/Linux-CI ist grün. Der geprüfte Mac-Build liegt unter `build_mac/uFlipper.app`, läuft regulär und besitzt die USB-Verbindung; der vorherige Build bleibt unter `build_mac/previous-c2eea85` erhalten.

Keine Gesamt-Kernabnahme und keine Veröffentlichung. Allgemeine NFC-Werkzeuge und PC/SC-Systemreader bleiben offen. Tickets bleiben bis zum regulären PR-Merge offen.
