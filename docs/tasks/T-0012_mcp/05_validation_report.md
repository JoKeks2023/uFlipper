# Validierung · 02.11.2026

Zwischenstand; kein vollständiger MCP-/A17-Gerätenachweis.

## Unabhängige Softwareverträge

- CI [41492518249](https://github.com/JoKeks2023/uFlipper/actions/runs/41492518249), Quellstand `054f17a`: Linux-MCP-Verträge und offizieller Python-SDK-Client 2.3.0 bestanden. Gesamtbuild/Mac-Prüfung noch laufend.
- Moderner `server/discover`/Liste/Aufruf mit Revision `2026-07-28`; erzwungener Legacy-Handshake mit `2025-11-25`. Beide über den produktiven C++-stdio-Relay und Gateway an einen **explizit synthetischen Host ohne Geräteverbindung**. [SDK-Ergebnis](evidence/linux-054f17a-sdk.json).
- Default gesperrt; private Datei-/Verzeichnisrechte; falsches Socket-Geheimnis abgelehnt; fehlerhaftes JSON; fehlende moderne Metadaten nicht aus früheren Anfragen übernommen; EOF beendet den Relay; Widerruf schließt bestehende Clients und sperrt Wiederverbindung; fremde Computerfreigabe ignoriert. [Wire-Client-Ergebnis](evidence/linux-054f17a-stdio.json).
- Ergebnisarchiv 13165595512 nach Download gegen SHA-256 `121954197ae39a77d39940223d29ceaa76b118c32294fd4485fd2c4c4239f5bc` geprüft.

## Noch offen

Native App und Freigabedialog auf Mac/Linux praktisch prüfen; echter Nutzerzugang erst nach bewusster Freigabe. Gemeinsame UI-/MCP-Geräteaufträge, Hardwarekonflikt, Stop und Widerruf am Flipper noch nicht beobachtet. Artefakttransport am nativen Katalog noch nicht praktisch geprüft. Fehlende NDEF-Testkarte, verschobener IR-Test und nur vorbereitete Readerinstallation bleiben unverändert. Keine neuen Geräteaktionen oder Nutzerfreigaben in diesem Turn.

## Weitere Nachweise · Quellstand 2b7fb11

- CI [41494130250](https://github.com/JoKeks2023/uFlipper/actions/runs/41494130250): vollständiger Linux-Desktoplauf sowie Kern Mac/Linux und Reader-Vertrag erfolgreich. Mac-Desktoplauf noch laufend.
- Beide MCP-Revisionen mit offiziellem Python-SDK 2.3.0 und unabhängigem Wire-Client auch im aktuellen Mac-CI-Lauf bestanden. [Mac-SDK](evidence/mac-ci-2b7fb11-sdk.json), [Mac-Transport](evidence/mac-ci-2b7fb11-stdio.json). Synthetischer Host ohne Gerätezugriff; keine Geräteabnahme daraus ableiten.
- Linux: 21 native Klicks, 16 Screenshots, 0 erfasste QML-Fehler, Easy-Mode-/Dropdown-Rundlauf und 1060×680. Freigabedialog öffnen, Fokus auf Abbrechen prüfen und mit Leertaste verwerfen; Freigabe bleibt aus. Screenshot geprüft und ursprünglich zu hoher Legacy-Dialog durch kompakten Werkzeugdialog ersetzt. [Ergebnis](evidence/linux-ui-2b7fb11.json).
- Tatsächlicher Flipper mit **unverändertem bisherigem Paket b010f9e** nur lesend vorgeprüft: Momentum mntm-012/e1784e74, f7; native Bereitschaft vorhanden, Vollbackup weiter gesperrt. Kein NFC-Feld, keine neue Installation und keine MCP-Aktivierung. [Befund](evidence/device-preflight-b010f9e.json). Dies ist kein Nachweis des neuen MCP-Backends.
- Der separat heruntergeladene Mac-Testhost aus 054f17a ist nur x86_64. Lokaler Start auf dem M1 scheiterte mit `Bad CPU type`; weder Rosetta installiert noch diesen Versuch als bestanden gezählt. Die lokale App-Prüfung erfolgt mit dem Universalpaket.

Geprüfte Downloadarchive: aktuelle Linux-UI 13165573444 `f6c7942f009dc2ce120e2c56191665a33b739ff7313ac234fc27b75e1b0d556a`; aktueller Mac-MCP-Vertrag 13166163185 `a871decfeb27cd63b5171e9751276fa7e78385e1df468a8f3e1578c8d4335ac7`.
