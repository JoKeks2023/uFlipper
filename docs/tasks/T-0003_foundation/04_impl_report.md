# Laufender Arbeitsbericht

27.10.2026: Branch angelegt; Planung auf 27 Entscheidungen ergänzt. Umsetzung läuft.

Implementiert: portable C++17-Kernmodule für gemeinsame Ressourcenbelegung, Konfliktentscheidung, Stop-Bestätigung und unbekannten Gerätezustand; SQLite-/Dateikatalog für fortlaufende Aufzeichnung, exklusiver Besitzer, Hashprüfung und Wiederauffinden nach Prozessabbruch; libsodium Secretstream-/Argon2id-Bausteine mit authentifizierter Veröffentlichung entschlüsselter Dateien.

Produktidentität und CI werden auf uFlipper ausgerichtet. Der Upstream-App-Updatekanal wird deaktiviert; Firmwarequellen bleiben getrennt. Neue CI erzeugt ausschließlich Testartefakte. Vollständige Cloud-Anbindungen, Schlüsselverwaltung, MCP und Systemreader sind damit noch nicht implementiert.

Befund: GitHub-Issues sind im Fork deaktiviert (`gh issue list`); der konkrete Updater-/CI-Bug konnte daher nicht als Issue erfasst werden. Die verbundene GitHub-Integration hat außerdem keine Schreibberechtigung für Issues. Keine Repo-Einstellungen eigenmächtig geändert.

Befund: bisheriger DMG-Dekorationspfad verweist auf einen nicht vorhandenen Background-Ordner. Der neue Testpaket-Ablauf verwendet eine einfache DMG mit Applications-Link.

## Nächster Zwischenstand

- Nativer serieller Empfangsprüfer mit demselben Belegungs-/Aufzeichnungskern ergänzt; begrenzte Lese-Puffer, exklusiver Port und bestätigter Ctrl+C-Stop.
- CI-Kernprüfung unter Ubuntu bestanden. Erster Workflow hatte einen YAML-Escapingfehler; korrigiert. Linux-Desktoplauf zeigte eine fehlende Suchwerkzeug-Abhängigkeit beim Setzen des Qt-Pfads; auf Python-Standardsuche geändert.
- macOS-Runner auf 15 umgestellt, da GitHub macOS 14 bis 02.11.2026 außer Betrieb nimmt; Deployment-Ziel bleibt macOS 13. Intel-Coreprüfung zusätzlich eingeplant. Quelle: https://github.com/actions/runner-images/issues/13518
- Drei visuelle Entwürfe vorbereitet und ausdrücklich als Beispieldaten gekennzeichnet; Produkt-UI noch nicht ersetzt.

## Empfang, App-RPC und geänderte Gestaltung

- Firmware-bestätigten Empfangsstart ergänzt. Momentum `subghz rx` braucht bei expliziter Frequenz zusätzlich den internen Geräteindex `0`; korrigiert. Früher CLI-Rücksprung beendet den Versuch als Fehler, statt einen erfolgreichen Empfang vorzutäuschen.
- Fehlerhafte/fehlende Aufnahme darf beim Wiederöffnen nicht den übrigen Katalog blockieren. Mit echtem Prozessabbruch und entferntem Sitzungsordner reproduziert und behoben; beschädigten Eintrag als unterbrochen ohne gültige Integritätsprüfung erhalten.
- App-RPC Start/Exit/DataExchange und Broadcast-Decodierung im bestehenden Qt-Gerätepfad ergänzt. Neue App-Operationen prüfen jede Fehlerantwort. Plugin-Interface-Version angehoben, damit alte Upstream-Plugins nicht mit verändertem Interface geladen werden.
- NFC-FAP 0.1.0 für das exakte Momentum-SDK gebaut. Quelle, Protokollgrenzen und Artifact-Hash unter `helpers/nfc_bridge/`; keine Installation oder Firmwareänderung.
- macOS-Desktop- und Core-CI auf verfügbaren Intel-Runner gelegt; Universal-Ziel bleibt Intel + Apple Silicon, lokaler ARM-Kerntest bestanden. ARM-GitHub-Runner warteten wiederholt; keine Plattform gestrichen.
- Neue Produktentscheidung 28: **gesamte App im qFlipper-Design**. Nutzer hat die drei hellen Entwürfe gesehen und abgelehnt. Neue Dashboard-/Werkzeugansichten nutzen das vorhandene Theme; größerer UI-Umbau weiterhin erst nach sichtbarer Prüfung.
