# Prüfkriterien

- [x] Mac-/Linux-Compile und fokussierte Protokoll-/Zugriffstests grün.
- [x] Unabhängiger stdio-Client prüft beide Protokolle einschließlich Fehlerfällen (synthetischer Host, Mac/Linux und lokaler M1).
- [x] Ohne Freigabe kein Geräte-/Aufnahmezugriff im Paket; falsches IPC-Geheimnis im produktiven Gateway mit Testhost abgelehnt.
- [x] Widerruf schließt Clients; neue Verbindung ohne Freigabe abgelehnt (produktiver Gateway, synthetischer Host; Geräte-Stop separat offen).
- [ ] UI und MCP zeigen dieselben Auftrags-IDs und Zustände.
- [ ] Hardwarekonflikt verändert die vorhandene Sitzung nicht.
- [ ] Große Artefakte begrenzt, keine freien lokalen Dateipfade.
- [x] Gebündelte App tatsächlich auf M1 ausführen; Nutzerfreigabe bleibt bis zur bewussten Zustimmung ausgeschaltet.
- [x] Beobachtete Ergebnisse und ausstehende Gerätebelege getrennt dokumentiert.
