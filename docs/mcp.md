# Lokaler MCP-Zugang

Entwicklungsstand, noch keine vollständige A17-Abnahme. Die Desktop-App bleibt der einzige Besitzer der Flipper-Verbindung. Der stdio-Prozess `uFlipper --mcp` reicht Anfragen an dieselbe laufende App weiter und startet keine zweite Geräteverbindung.

## Einrichten und beenden

1. uFlipper öffnen und unter **Einstellungen > Integrationen > MCP freigeben …** die Gesamtfreigabe auf diesem Computer ausdrücklich bestätigen.
2. **Client-Konfiguration anzeigen** öffnen. `command` und `args` in die Konfiguration des gewünschten MCP-Clients übernehmen; der absolute Anwendungspfad steht bereits darin.
3. Die Desktop-App während der Nutzung geöffnet lassen. Verbundene Clients erscheinen in den Einstellungen; ihre Namen sind selbst gemeldete Anzeigenamen.
4. **Freigabe widerrufen** schließt alle MCP-Verbindungen und fordert den Stop eines laufenden MCP-NFC-Auftrags an. Einen bestätigten Stop im NFC-Werkzeug kontrollieren. Ein unbestätigter Stop bleibt als solcher gesperrt.

Die Freigabe gilt für alle lokal verfügbaren MCP-Aktionen und bleibt beim Neustart auf demselben Computer erhalten. Ohne Freigabe beendet sich der stdio-Prozess ohne Gerätedaten. Freigabedatei und wechselndes Zugangsgeheimnis liegen außerhalb der Workbench-Ablage unter dem lokalen Konfigurationspfad `uFlipper/local-mcp/<Computerkennung>/`; sie gehören nicht in Synchronisation oder Export. Eine kopierte Freigabe eines anderen Computers aktiviert den Zugang nicht.

Ein geschlossenes Clientprogramm beendet seine Verbindung. Bereits gestartete, separat identifizierte Geräteaufträge bleiben in der App sichtbar und zeichnen weiter auf; sie werden über `job_stop`, den UI-Stop oder Widerruf beendet. Stdio-Anfragen werden nicht automatisch wiederholt. Ein Verbindungsfehler beweist weder Erfolg noch Fehlschlag einer schon gestarteten Geräteaktion: anschließend Auftragsstatus prüfen.

## Aktuelle Werkzeuge

| Werkzeug | Verhalten |
| --- | --- |
| `capabilities` | Angeschlossene Funktionen, Voraussetzungen und explizit offene Gerätebereiche. |
| `device_status` | Gemeinsamer Geräte-/Firmware-/Akku-/Power-/Speicherstand mit Verfügbarkeits- und Veraltet-Markierungen. |
| `jobs_list` | Aktuelle NFC-Sitzung, letzte 40 NFC-Aufträge und Backupzustand. UI und MCP verwenden dieselben IDs. Noch kein vollständiges Auftragsregister für sämtliche Gerätebereiche. |
| `nfc_start` | `mode`: `ppse` oder `ndef`. Startet dieselbe dauerhaft gespeicherte Lesesitzung wie die UI und liefert sofort einen Auftragsbezug; Abschluss über `jobs_list` prüfen. |
| `job_stop` | `jobId` des aktuellen NFC-Auftrags. Fordert expliziten Stop an; `stopConfirmed` unterscheidet Anforderung und Bestätigung. |
| `recordings_list` | Letzte 40 Einträge desselben Aufnahmekatalogs. Abgeschlossene/unterbrochen abgeschlossene Einträge enthalten Artefaktverweise. |
| `artifact_read` | `uri` aus der Aufnahmeliste; `offset` ab 0, `limit` 1–32768 Byte, Standard 16384. Base64-Chunk mit Gesamtgröße, nächstem Offset und EOF. Keine freien lokalen Dateipfade. |
| `backup_status` | Lokaler Sicherungskatalog und weiterhin wirksames Vollbackup-Gate. |
| `firmware_status` | Weiterhin wirksame Update-/Installations-/Restore-Voraussetzungen; noch keine Firmwarebibliothek. |

Bei belegter oder unbestätigt gestoppter Hardware liefert `nfc_start` einen Werkzeugfehler `hardware_conflict`; die vorhandene Sitzung bleibt unverändert. Status prüfen, bewusst den bezeichneten Auftrag stoppen und erst nach bestätigtem Ende erneut starten. Die Gesamtfreigabe ersetzt weder diese Konfliktentscheidung noch die separate Installationsfreigabe jeder Helferversion. MCP installiert keinen Helfer und umgeht keine Backup-Voraussetzung.

NFC benötigt weiterhin genau den bereits freigegebenen Helfer 0.1.0 auf dem dafür geprüften Momentum-Stand. Die NDEF-Geräteprüfung und der unabhängige Mac-Systemreader-Nachweis fehlen weiterhin; MCP macht sie nicht automatisch vollständig verifiziert.

## Protokoll und Grenzen

- `2026-07-28`: `server/discover`, Versions-/Fähigkeitsmetadaten pro Anfrage, `resultType`, private Listencaches mit TTL 0. Keine Initialisierung erforderlich.
- `2025-11-25`: `initialize`, danach `notifications/initialized` und normale Werkzeugaufrufe.
- Ausschließlich lokaler stdio-Zugang; interne Verbindung über authentifizierten Unix-Socket. Keine HTTP-/TCP-Freigabe.
- 64 KiB pro Eingangsnachricht, 256 KiB Transportpuffer, maximal acht lokale Clientverbindungen; keine JSON-RPC-Batches.
- Werkzeugaufrufe liefern unmittelbar Status/Job-Handle. `notifications/cancelled` für bereits beantwortete Anfragen beendet keinen eigenständig laufenden Geräteauftrag; dafür ist `job_stop` vorgesehen.
- Noch offen: MCP-Dateitransfer, weitere Gerätewerkzeuge, Firmwarebibliothek, Cloud und volle A17-Prüfung. Es werden ausschließlich implementierte Werkzeuge angeboten.

Validierung und praktische Grenzen: [T-0012](tasks/T-0012_mcp/05_validation_report.md).
