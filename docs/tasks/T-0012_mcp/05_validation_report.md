# Validierung · 02.11.2026

**Aktueller Paketstand `ed9650058746f5562547460df292ec12783f0587`: alle fünf CI-Jobs grün, Universalpaket auf dem lokalen M1 ausgeführt, native Oberfläche und Gerätestatus geprüft. MCP-Nutzerfreigabe bleibt aus. Der echte MCP-/Flipper-Vergleich wartet auf die bewusste temporäre Testfreigabe; A17 ist nicht vollständig abgenommen.**

## Geprüftes Produktpaket

- CI [41499931808](https://github.com/JoKeks2023/uFlipper/actions/runs/41499931808): Desktop Mac/Linux, Kern Mac/Linux und Reader-Vertrag ohne Registrierung erfolgreich. Frühe Geräte-/Aufnahme-/Backupverträge, unabhängiger MCP-Client und Paketstarts bestanden. Stdio-Start bleibt ohne Freigabe gesperrt, auch bei absichtlich ungültiger GUI-Plattform.
- Mac-Artefakt 13169552193: Archiv-SHA-256 `8a2d6cacebf148ed1b76af9198b6cdcbbb1b1952fcd42583b3c15a82cf84c04f`; Binary-SHA-256 `abceca9e94f72515edae376b5f346b8cffdbf73c897cbedb40deed90c56fcaf0`. Universal x86_64/arm64, Bundle-ID `com.jorisconrad.flipperzeroutil`, strenge Codesignprüfung erfolgreich. [Paketbefund](evidence/mac-package-ed96500.json).
- Aktuelles Paket unter `build_mac/uFlipper.app`; Vorversion unter `build_mac/previous-before-mcp-ed96500/uFlipper.app` erhalten. Keine öffentliche Veröffentlichung und kein Merge.

## Tatsächlich auf dem lokalen M1 mit Flipper beobachtet

- Normale Produktoberfläche: 21 Klickprüfungen, 16 Screenshots, 0 erfasste QML-Fehler. Easy Mode und Einstellungs-Dropdown hin/zurück; kompakte Ansicht 1060×680; MCP-Freigabedialog öffnen, Fokus auf Abbrechen prüfen und per Leertaste verwerfen. Freigabe bleibt aus. Dashboard und Dialog selbst visuell geprüft. [UI-Ergebnis](evidence/mac-ui-ed96500.json).
- Echter stdio-Paketstart `--mcp` ohne Freigabe: Exit 2, leeres stdout, verständlicher Hinweis auf die fehlende Freigabe; keine zweite GUI-Instanz.
- Anschließend Original-Produktfenster mit lokalem Diagnosebeobachter: gemeinsamer Backend bereit, Momentum mntm-012/e1784e74, NFC idle/startfähig, MCP aus, keine Clients, Vollbackup weiterhin gesperrt. Keine NFC-Aktivierung und keine Nutzerfreigabe. Das ist ein Backend-Diagnosenachweis und kein MCP-Gerätetest. [Gerätebefund](evidence/mac-device-ed96500.json).
- Lokale Originale einschließlich Screenshots und Logs: `build_mac/mcp-ed96500/`. Private Gerätedetails bleiben außerhalb des Repos.

## Unabhängige MCP-Kommunikation

- Beide Revisionen mit offiziellem Python-SDK 2.3.0: moderner `server/discover`/Liste/Aufruf mit 2026-07-28 und erzwungener Legacy-Handshake mit 2025-11-25. Produktiver C++-Relay/Gateway, **ausdrücklich synthetischer Host ohne Gerätekommunikation**.
- Default gesperrt; private Datei-/Verzeichnisrechte; falsches Zugangsgeheimnis abgelehnt; fehlerhaftes JSON; moderne Metadaten nicht aus früheren Anfragen übernommen; EOF beendet den Relay; Widerruf schließt Clients und sperrt Wiederverbindung; fremde Computerfreigabe ignoriert.
- Linux 054f17a: [Wire](evidence/linux-054f17a-stdio.json), [SDK](evidence/linux-054f17a-sdk.json). Mac-CI 2b7fb11: [Wire](evidence/mac-ci-2b7fb11-stdio.json), [SDK](evidence/mac-ci-2b7fb11-sdk.json). Neuer geprüfter Paketlauf wiederholt diese Verträge erfolgreich.
- Zusätzlich tatsächlich lokal auf dem M1: Universal-Testadapter 3168f11 mit vorhandenen Qt-6.8.3-Frameworks. Eigenständige C++-Verträge sowie [Wire-Client](evidence/m1-3168f11-stdio.json) und [offizieller SDK-Client](evidence/m1-3168f11-sdk.json) bestanden. Keine MCP-Aktivierung im Nutzerprofil und kein Gerätezugriff dieses Hosts.
- Zu Vergleichszwecken: [Linux-UI 2b7fb11](evidence/linux-ui-2b7fb11.json), ebenfalls 21 Klicks/16 Screenshots/0 erfasste QML-Fehler. [Lesende Gerätevorprüfung mit Vorpaket b010f9e](evidence/device-preflight-b010f9e.json) ist separat ausgewiesen und wird nicht als Nachweis des neuen MCP gezählt.

## Gefundene Fehler und nicht bestandene Versuche

- [#15](https://github.com/JoKeks2023/uFlipper/issues/15): neue Python-Testeinrichtung überschrieb zunächst den Universal-Package-Pfad. Lauf 41494130250 scheiterte dadurch beim ARM-Linken. Python nun vor Plattformabhängigkeiten eingerichtet; tatsächliche Package-Prefixe und beide Archivarchitekturen vor der Kompilierung und dem Vollbuild geprüft. Aktueller Gesamtbuild erfolgreich; Issue bleibt bis Merge offen.
- Der erste zusätzliche Architektur-Guard verwendete `lipo`-Argumente unpassend. Lauf 41498058961 wurde früh abgebrochen und gilt nicht als fertiges Paket. Portable Korrektur prüft die Eingabedatei zuerst und jede Architektur einzeln; vollständiger Shell-Guard lokal an realer Universaldatei positiv sowie mit falschem Prefix und fehlender Architektur negativ ausgeführt. Danach im aktuellen CI-Lauf an den echten statischen Abhängigkeiten bestanden.
- Ein ursprünglich nur für Intel gebauter Testhost 054f17a ließ sich auf dem M1 nicht starten (`Bad CPU type`). Kein Rosetta installiert und nicht als bestanden gezählt. Der spätere Universal-Testhost und die Universal-App wurden tatsächlich auf dem M1 ausgeführt.
- Zu hoher erster Freigabedialog anhand echter Linux-Screenshots erkannt und durch den kompakten Werkzeugdialog ersetzt; danach Linux- und Mac-UI erneut bestanden.

## Offen

Echter unabhängiger MCP-Client am produktiven Flipper: temporäre Gesamtfreigabe, Status/Artefakt-Chunks, derselbe UI-/MCP-Auftrag, Hardwarekonflikt ohne stillen Stop, bestätigter Stop und Widerruf eines MCP-Auftrags. Nur nach bewusster Testfreigabe; NFC-Start/Stop zusätzlich erst bei bestätigter freier Rückseite. Keine automatische Zustimmung durch Wartezeit.

Voller MCP-Werkzeugumfang, Cloud-Import am zweiten Computer und A17 bleiben offen. Fehlende NDEF-Testkarte, verschobener IR-Test und nur vorbereitete Mac-Readerinstallation unverändert. Kein vollständiges Gerätebackup, Restore oder Firmwareupdate nachgewiesen; bestehende Sperren erhalten. Keine Helfer-/Firmwareänderung und keine öffentliche Kernabnahme.
