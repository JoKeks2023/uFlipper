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
