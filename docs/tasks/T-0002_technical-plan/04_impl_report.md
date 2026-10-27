# Arbeitsbericht · ausschließlich Planung

27.10.2026:

- Lokalen Fork lesend geprüft, Ausgangsrevision und Arbeitsbaum dokumentiert.
- 51 Momentum-Quelldateien am festgelegten Commit sowie 17 Dateien des Forks geprüft; Quellenpfade und Hashes erfasst.
- Öffentliche Primärdokumentation und Release-/Update-Metadaten lesend geprüft. Keine Firmwarepakete bezogen.
- 65 Funktions-/Zielzeilen mit Stand, Geräteweg, Host-Arbeit, Ressourcen, Datenprofil, Belegen und Abnahmereferenz erstellt.
- Betriebsmodell, Firmware-/Backup-/Cloud-Abläufe, UI-Abläufe und Umsetzungs-/Abnahmereihenfolge vorgeschlagen.
- T-0001 um Entscheidung 16 und Verweise auf diesen technischen Entwurf ergänzt. Frühere Prüfergebnisse bleiben als historischer Stand erkennbar.

Änderungen liegen ausschließlich in `docs/tasks/T-0001_product-plan/` und `docs/tasks/T-0002_technical-plan/`. Keine Anwendungscodeänderung, Installation, Gerätebedienung, Übertragung von Nutzerdaten oder Veröffentlichung. Kein Commit und kein Push.

Eine gemeinsame Planabnahme und alle Laufzeitnachweise stehen noch aus. [05_validation_report.md](05_validation_report.md) trennt Dokumentprüfung von Funktionsprüfung.

## Weitere Auswahlrunde · 27.10.2026

Entscheidungen 17–22 sind ausdrücklich bestätigt: feste Seitenleiste als Startansicht; Update/Wechsel/Restore bis zu einem geprüften Backup anhalten; Google Drive, Dropbox, OneDrive, WebDAV und S3 bereits im ersten Release; Löschungen überall in den Papierkorb und endgültig nur manuell; Wiederherstellungsschlüssel und Masterpasswort als wählbare Zugangswege für verschlüsselte Daten; jede neue Helferversion erneut ausdrücklich freigeben. Alle drei Layouts bleiben auswählbar, automatische Bereinigung bleibt zunächst aus.

Dies konkretisiert Produktverhalten. Es startet keine Implementierung, Cloud-Verbindung, Installation oder Geräteaktion.

Produkt-Scope, technische Restfragen, CSV/Markdown-Matrix, Quellenhinweise und Abnahmekriterien entsprechend aktualisiert. Die bestehenden Quellenrevisionen und der unverifizierte Laufzeitstand bleiben erhalten.
