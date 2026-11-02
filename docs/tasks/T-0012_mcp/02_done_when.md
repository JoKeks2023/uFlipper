# Prüfkriterien

- [ ] Mac-/Linux-Compile und fokussierte Protokoll-/Zugriffstests grün.
- [ ] Unabhängiger stdio-Client prüft beide Protokolle einschließlich Fehlerfällen.
- [ ] Ohne Freigabe kein Geräte-/Aufnahmezugriff; falsches IPC-Geheimnis abgelehnt.
- [ ] Widerruf schließt Clients; neue Verbindung ohne Freigabe abgelehnt.
- [ ] UI und MCP zeigen dieselben Auftrags-IDs und Zustände.
- [ ] Hardwarekonflikt verändert die vorhandene Sitzung nicht.
- [ ] Große Artefakte begrenzt, keine freien lokalen Dateipfade.
- [ ] Gebündelte App tatsächlich ausführen; Freigabe des echten Nutzerprofils nur nach bewusster Zustimmung.
- [ ] Beobachtete Ergebnisse und ausstehende Gerätebelege getrennt dokumentiert.
