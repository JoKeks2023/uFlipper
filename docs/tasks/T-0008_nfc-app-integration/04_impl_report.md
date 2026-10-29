# Umsetzung

- GitHub-Arbeitsticket: [#3](https://github.com/JoKeks2023/uFlipper/issues/3), In Progress. Huly ist in dieser Sitzung nicht verbunden.
- `NfcSession` verwaltet Start/ACK/Broadcast/Antwort/Stop, Zeitgrenzen und lokale Originalaufnahmen über CaptureStore/OperationCoordinator. Start ohne beschreibbare Ablage wird abgewiesen.
- `NfcController` verwendet ausschließlich DeviceRegistry/ProtobufSession der App. Prüft Firmwareziel, Größe und SHA-256 des bereits freigegebenen Helfers; keine Installation.
- NFC-Seite im vorhandenen qFlipper-Raster: Start/Stop, Kartenstatus, feste PPSE-Auswahl, Antwortdaten und gespeicherte Sitzungen.
- Konkurrierende Bildschirm-/Datei-/Firmwareansichten fragen vor dem Stop nach und warten auf Bestätigung. Backend-Sperren verhindern parallele Eingaben und Dateiaktionen auch unabhängig von der Navigation.
- Native Prüfoptionen `--review-nfc DIR` und optional `--review-nfc-card` klicken die echte Oberfläche. Kein automatischer Gerätetest in CI und kein Simulator.

Validierung läuft; Umsetzung noch nicht abgenommen.
