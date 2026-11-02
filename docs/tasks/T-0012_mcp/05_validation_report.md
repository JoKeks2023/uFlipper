# Validierung · 02.11.2026

Zwischenstand; kein vollständiger MCP-/A17-Gerätenachweis.

## Unabhängige Softwareverträge

- CI [41492518249](https://github.com/JoKeks2023/uFlipper/actions/runs/41492518249), Quellstand `054f17a`: Linux-MCP-Verträge und offizieller Python-SDK-Client 2.3.0 bestanden. Gesamtbuild/Mac-Prüfung noch laufend.
- Moderner `server/discover`/Liste/Aufruf mit Revision `2026-07-28`; erzwungener Legacy-Handshake mit `2025-11-25`. Beide über den produktiven C++-stdio-Relay und Gateway an einen **explizit synthetischen Host ohne Geräteverbindung**. [SDK-Ergebnis](evidence/linux-054f17a-sdk.json).
- Default gesperrt; private Datei-/Verzeichnisrechte; falsches Socket-Geheimnis abgelehnt; fehlerhaftes JSON; fehlende moderne Metadaten nicht aus früheren Anfragen übernommen; EOF beendet den Relay; Widerruf schließt bestehende Clients und sperrt Wiederverbindung; fremde Computerfreigabe ignoriert. [Wire-Client-Ergebnis](evidence/linux-054f17a-stdio.json).
- Ergebnisarchiv 13165595512 nach Download gegen SHA-256 `121954197ae39a77d39940223d29ceaa76b118c32294fd4485fd2c4c4239f5bc` geprüft.

## Noch offen

Native App und Freigabedialog auf Mac/Linux praktisch prüfen; echter Nutzerzugang erst nach bewusster Freigabe. Gemeinsame UI-/MCP-Geräteaufträge, Hardwarekonflikt, Stop und Widerruf am Flipper noch nicht beobachtet. Artefakttransport am nativen Katalog noch nicht praktisch geprüft. Fehlende NDEF-Testkarte, verschobener IR-Test und nur vorbereitete Readerinstallation bleiben unverändert. Keine neuen Geräteaktionen oder Nutzerfreigaben in diesem Turn.
