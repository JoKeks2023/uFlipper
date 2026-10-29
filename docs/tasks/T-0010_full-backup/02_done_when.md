# Prüfkriterien

- [ ] Vollbackup verwendet den bestehenden RPC-Verbindungsbesitzer und sperrt konkurrierende Geräteaktionen.
- [ ] Alle Dateien einschließlich leerer Dateien/Verzeichnisse in beiden Scopes; keine vollständige Sicherung bei fehlender SD oder geändertem Inventar/Inhalt.
- [ ] Dateiinhalte werden mit begrenztem RAM geschrieben; Prüfung blockiert die Oberfläche nicht.
- [ ] Abbruch, Verbindungsabbruch und bestätigter Abschluss bleiben unterscheidbar; Teilbackups wieder auffindbar.
- [ ] Native UI am Gerät tatsächlich ausgeführt und Backupdaten unabhängig geprüft.
- [x] Mac-/Linux-CI und gezielte Verträge erfolgreich (b010f9e; tatsächliche Vollbackup-Abnahme bleibt wegen des Inventargates offen).

Konkretes offenes Gate: #13 / ungefiltertes, fehlerbewusstes Geräteinventar. Erfolgreiche kontrollierte Inventarverträge ersetzen diesen Gerätenachweis nicht.
