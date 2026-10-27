# Randbedingungen und Arbeitsgrenzen

Status: Diese Grenzen dokumentieren die abgeschlossene Planungsrunde. Der Nutzer hat anschließend den verbindlichen Umsetzungsplan ausdrücklich beauftragt; für die Umsetzung gelten [T-0003](../T-0003_foundation/00_brief.md) und die dort dokumentierten Freigabegrenzen.

- Diese Aufgabe ist Produktplanung. Keine Anwendungscodeänderungen, Geräteaktionen, Installationen, Cloud-Verbindungen, GitHub-Änderungen, Commits oder Pushes als Teil dieser Dokumentation.
- Eigene Hilfsprogramme/Skripte auf SD sind nach ausdrücklicher Freigabe erlaubt (27.10.2026). Diese Entscheidung erlaubt keine jetzige Installation.
- Erstes Ziel: unverändertes Momentum. Gewünschte Funktionen dürfen nicht als möglich dargestellt werden, bevor ihr Steuer- und Datenweg geprüft ist.
- IR/NFC sind Beispiele für alle Gerätebereiche. Keine stillschweigende Beschränkung auf diese beiden Module oder auf einen Dateibrowser.
- Messgrößen korrekt benennen: Der eingebaute IR-Empfänger liefert demodulierte Pulsinformationen; das ist kein optisches Frequenzspektrum und keine analoge Signalstärkemessung.
- NFC-Auswertung innerhalb der App und ein von anderer Mac-Software nutzbarer Reader sind getrennte Nachweise.
- Die bestehenden Cloud-, Live- und MCP-Auswahlen beschreiben zukünftiges Produktverhalten. Sie aktivieren heute keine Übertragung, Aufzeichnung oder Gerätebedienung.
- macOS und Linux, kompakte sachliche Gestaltung, drei wählbare Layoutmodi.
- M1/8 GB: keine Simulatoren/Emulatoren, schwere lokale Builds vermeiden; spätere Build-Prüfungen möglichst in CI.
- Enumerierbare UI-Werte als Auswahl statt Freitext; echte Sonderfälle bei Bedarf über „Andere…“.
- Keine Squash-Merges. Bisherige eigenständige Python-App und deren Release-Arbeit werden durch diese Planung nicht fortgesetzt.
- Die vorhandene qFlipper-Lizenz und Upstream-Hinweise bleiben Grundlage.
