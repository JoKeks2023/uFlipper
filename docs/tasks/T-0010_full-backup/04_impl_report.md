# Umsetzung

GitHub [#8](https://github.com/JoKeks2023/uFlipper/issues/8), In Progress; leerer RPC-Dateitransfer [#9](https://github.com/JoKeks2023/uFlipper/issues/9). App-Quellstand `807e62e`.

- `BackupController` nutzt ausschließlich bestehende DeviceRegistry/RPC-Verbindung. Kein zweiter USB-Besitzer. Dateioperationen, NFC, Bildschirmbedienung und Telemetrie werden während einer Vollsicherung gesperrt beziehungsweise angehalten.
- `BackupSession`: beide Speicher vorhanden prüfen; rekursives Inventar einschließlich leerer/verborgener Einträge; Inhalte fortlaufend schreiben; Remote-MD5 nach jedem Transfer; abschließendes zweites Inventar und erneuter Remote-Inhaltsvergleich. Keine atomare Dateisystem-Momentaufnahme verfügbar. Bei Änderungen/Fehlern kein Vollständigkeitserfolg.
- RAM: keine Dateiinhalte puffern; Inventar auf 200.000 Einträge begrenzt. SHA-256 beim Schreiben berechnen, lokale Abschlussprüfung in einem Worker.
- Abbruch schließt den bestehenden RPC-Transport, weil die Schnittstelle keinen einzelnen Dateitransfer abbrechen kann. Späte Antworten starten keine Folgeaktionen; Teilbackup bleibt erhalten. Vor nächster Geräteaktion USB neu verbinden.
- Native Backup-Seite mit Fortschritt, Konfliktentscheidung und lokalem Katalog; Dashboard verweist auf Status/Katalog. Kein Gestaltungswechsel.
- `scripts/backup/verify_backup.py` prüft fertige Sicherungen unabhängig und nur lesend mit Python/SQLite/SHA-256.
- Bestehende Firmware-/Restore-Sperre bleibt: ein manuelles Vollbackup ersetzt keinen frischen, automatischen Sicherheitsablauf samt Kompatibilitätsprüfung.

Reader-Installation, fehlende NDEF-Testkarte und verschobener IR-Test werden nicht zum pauschalen Stopp der übrigen Umsetzung. Website-Arbeit unverändert.

- Abbruchbedingter RPC-Altfehler [#10](https://github.com/JoKeks2023/uFlipper/issues/10): bereits eingeplanter Queue-Callback darf einen geschlossenen Transport nicht wieder als Idle markieren. Guard und reproduzierbarer Callback-Test ergänzt.
- `/int` kann bei Momentum auf SD liegen (bereits unter T-0004 belegt). Umfang in Oberfläche/Metadaten ausdrücklich auf RPC-Dateibereiche begrenzt, kein Rohabbild der physischen Firmware behauptet.
