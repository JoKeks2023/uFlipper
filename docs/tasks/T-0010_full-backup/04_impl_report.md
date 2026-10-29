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

## Konkrete Inventarvoraussetzung – Vollbackup bleibt gesperrt

[#13](https://github.com/JoKeks2023/uFlipper/issues/13): Die [geprüfte Momentum-Quelle](https://github.com/Next-Flip/Momentum-Firmware/blob/e1784e7418d8b074e971983ceb6fef0f37e52ae4/applications/services/rpc/rpc_storage.c#L218) filtert nicht-ASCII-Dateinamen aus `storage.list`. Zwei solche Listen können übereinstimmen und trotzdem SD-Inhalte auslassen. Ein reiner RPC-Adapter darf deshalb kein vollständiges Gerätebackup bestätigen. Der native Start bleibt bis zu einem ungefilterten, fehlerbewussten Inventar gesperrt; UI und Protokolltest benennen die Voraussetzung. Keine Umbenennung einer Teilsicherung zu einem Vollbackup.

Geplanter nächster Adapter: lesender SD-Inventarhelfer über den gemeinsamen App-RPC-Besitzer; bytegenaue Namen und eindeutiges Verzeichnisende/Lesefehler. Zunächst vorzubereiten und separat zu prüfen. Jede Installation/Version braucht die vereinbarte ausdrückliche Freigabe. Bisher kein neuer Helfer gebaut oder installiert. Die unveränderte Momentum-Firmware und der NFC-Helfer bleiben erhalten. Host-Speicher-/Verschlüsselungsarbeit läuft unabhängig weiter (T-0011).

Zusätzlicher SDK-Befund: API 87.1 exportiert `storage_file_get_error`, aber `storage_file_get_internal_error` ist in der verifizierten `api_symbols.csv` ausdrücklich nicht exportiert (`Function,-,...`). Die gepinnte FAT-Verzeichnisschicht setzt bei leerem Namen FSE_NOT_EXIST; ein Helfer muss daher die belastbare Unterscheidung von Ende/Lesefehler zuerst nachweisen. Es liegt noch kein geprüfter Inventarhelfer und keine Installationsanfrage vor. Kein Firmwareumbau als automatische Umgehung.
