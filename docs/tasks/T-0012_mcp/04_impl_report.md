# Umsetzung

In Arbeit. [Nutzung und Werkzeugschnittstelle](../../mcp.md).

- Das normale App-Binary erkennt `--mcp` vor QApplication/Einzelinstanz-/Geräteregistrierung und startet ausschließlich ein QtCore-stdio-Relay.
- Native `ApplicationBackend`-Instanz besitzt den lokalen Gateway. 256-Bit-Geheimnis pro App-Lauf, private Discovery-/Freigabedateien, maschinengebundene lokale Freigabe, maximal acht Clients, begrenzte Eingabe/Schreibqueues. Kein TCP-Port.
- Zwei getrennte Protokollpfade: modernes `server/discover` mit Pflichtmetadaten je Anfrage, bzw. Legacy-Initialisierung. Keine Behauptung, ein Legacy-Handshake genüge für die moderne Revision.
- Neun Werkzeuge. NFC startet und stoppt denselben `NfcController` wie die UI. Ursprung/ID werden vor dem ersten Gerätekommando in die Originalaufnahme geschrieben und beim Wiederöffnen rekonstruiert. Status zeigt letzten NFC-Verlauf begrenzt auf 40 Einträge; ein vollständiges globales Register aller Module bleibt offen.
- Artefakte ausschließlich für im Katalog abgeschlossene/unterbrochen abgeschlossene Aufnahmen; feste URI-Struktur und Dateinamen, höchstens 32 KiB pro Chunk. Kein zweiter schreibender CaptureStore zum Lesen.
- Einstellungen: bewusster Gesamtfreigabedialog mit Abbrechen als Vorauswahl, aktive selbst gemeldete Clientnamen, Widerruf und kopierbare lokale Client-Konfiguration. App-Neustart erhält die Freigabe nur auf demselben Computer. Keine synchronisierbare Preferences-Eigenschaft zur stillen Aktivierung.
- Widerruf schließt Clients sofort und fordert Stop eines aktiven MCP-NFC-Auftrags an. Anforderung/Bestätigung bleiben getrennt. Normales Ende einer stdio-Verbindung lässt bereits gestartete, separat identifizierte Aufträge sichtbar weiterlaufen.
- Vorhandene Vollbackup-/Update-/Restore-Gates bleiben bestehen; keine Helfer-/Readerinstallation.

Noch kein vollständiger Produkt-/A17-Funktionsnachweis. Reale Geräteprüfung, offene Werkzeuge und Cloud-Import separat prüfen.

Quellen: [MCP 2026 Versionierung](https://github.com/modelcontextprotocol/modelcontextprotocol/blob/main/docs/specification/2026-07-28/basic/versioning.mdx), [2026 stdio](https://github.com/modelcontextprotocol/modelcontextprotocol/blob/main/docs/specification/2026-07-28/basic/transports/stdio.mdx), [2026 Schema](https://github.com/modelcontextprotocol/modelcontextprotocol/blob/main/schema/2026-07-28/schema.ts), [2025 Lifecycle](https://modelcontextprotocol.io/specification/2025-11-25/basic/lifecycle).
