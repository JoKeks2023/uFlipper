# Validierung

## Geprüfter Stand am 29.10.2026

- Implementierung: `be2a11593fedefcfd86644a5427b592635f60e79` auf `codex/uflipper-foundation`.
- [CI 40875576511](https://github.com/JoKeks2023/uFlipper/actions/runs/40875576511): alle vier Jobs erfolgreich. Portable Kernprüfungen auf Linux/Intel-Mac; Qt-App-RPC- und NFC-Vertragstests; Mac-Universal- und Linux-App gebaut und tatsächlich gestartet.
- Linux-Navigation: **18 Klicks, 14 Screenshots, 0 erfasste QML-Fehler**, einschließlich NFC-Seite, Easy Mode und Settings-Dropdown. Kein Gerät am CI-Runner. [Linux-Evidenz](evidence/linux-ui-be2a115.json)
- Mac-Paket: Universal `x86_64 arm64`, Bundle-ID `com.jorisconrad.flipperzeroutil`, strenge Codesign-Prüfung erfolgreich. CI-Archiv SHA-256 `d4e3e5e80cb3f6f3748f8f38a39eb85bfaa642bf10d82d69908360631ba5de95`.

## Echte native Mac-App mit angeschlossenem Flipper

Momentum `mntm-012`, Commit `e1784e74`, Ziel `f7`, RPC 0.25. Bereits freigegebener Helfer 0.1.0 unverändert; Datei vor jedem Start geprüft. Keine Installation und keine Firmwareänderung.

| Ablauf | Beobachtung | Aufnahme |
| --- | --- | --- |
| Ohne Karte, 13:06:59 UTC | 8 echte UI-Klicks: Start, Suche, Wechsel zum Bildschirm, „Weiterlesen“ lässt Messung laufen, Wechsel zu Dateien mit „Stoppen und wechseln“ wartet auf bestätigten Stop, Aufnahme erneut öffnen; 0 QML-Warnungen | `9ec3d5d1ffeb7a63c5f317b0baa6cd05`, 321 Bytes, Nutzer-Stop bestätigt, Integrität geprüft |
| Apple Pay, 13:08:48 UTC | Nach Nutzerfreigabe aus der App gestartet; ISO14443-4A/ISO-DEP aktiviert; zweimal festes SELECT PPSE, jeweils 46 Bytes und Status `9000`; Deaktivierung erfolgreich, AppExit und AppClosed beobachtet; gespeicherte Sitzung per UI wieder geöffnet; 0 QML-Warnungen | `84876db145e5aa53b5c946feb79363ca`, 502 Bytes, vollständig abgeschlossen, Integrität geprüft |

Originaldateien zusätzlich unabhängig gegen ihre Manifest-Prüfsumme verglichen. Die Klickfolge und die Metadaten aller Originalereignisse stehen in der [bereinigten Mac-Evidenz](evidence/mac-nfc-be2a115.json). Karten-UID, Antwortinhalte und Screenshots bleiben lokal. Die Apple-Pay-Beobachtung ist ein Verzeichnis-Lesetest, keine Zahlung und kein allgemeiner Kartendump.

Geprüfter Build nach `build_mac/uFlipper.app` übernommen; Binärdatei stimmt mit der getesteten überein. Regulärer Start und Übernahme des angeschlossenen USB-Ports beobachtet. Vorheriger Build unter `build_mac/previous-c2eea85` erhalten.

## Automatisierte Fehler- und Zustandsprüfungen

Qt-Verträge prüfen Start erst nach angelegter Aufnahme; ACK/AppStarted und AppExit/AppClosed in unterschiedlicher Reihenfolge; alte Transportbestätigung; Nutzer-Stop; Vorprüfungsabbruch; Verbindungsverlust und neue Generation; PPSE-Fehler; fehlende Antwort und ausbleibende Stopbestätigung; erneutes Öffnen mit identischen Antwortzeitpunkten; beschädigte Aufnahme; nicht beschreibbare Ablage. Unveränderte Verbindung bleibt ohne neue Zustandsmeldung; beobachtetes `f7` wird akzeptiert, falsche Firmware/anderes Ziel werden abgewiesen.

Diese Fehlerfälle wurden nicht sämtlich physisch erzwungen. Insbesondere wurde der freigegebene Helfer nicht entfernt oder verändert, um fehlende SD-/Helferdateien am Gerät zu testen.

## Frühere Fehlversuche und Korrekturen

`77bc7bb`: CI vollständig grün, realer Mac-Klicklauf scheiterte vor Helferstart. Unveränderte RPC-Idle/Running-Wechsel lösten neue Telemetrie aus (3.698 Kommandos in ca. 73 Sekunden); anschließend zeigte eine lokale Beobachtung die falsche Zielprüfung `7` statt `f7`. Beides in `be2a115` mit Regressionstests korrigiert und mit neuem CI-Paket erneut am Gerät geprüft. [Bug #4](https://github.com/JoKeks2023/uFlipper/issues/4), [Bug #5](https://github.com/JoKeks2023/uFlipper/issues/5). Der alte CI-Erfolg wird nicht als Funktionsnachweis verwendet.

## Offener Gesamtumfang

Allgemeines Kartenlesen, NDEF, mehrere voneinander abhängige Kartenkommandos und PC/SC-Systemreader sind nicht fertig. Der bisherige Apple-Pay-Nachweis ersetzt die Abnahme mit unabhängiger Mac-Anwendung und geeigneter Testkarte nicht. Kein Release und keine Gesamt-Kernabnahme.
