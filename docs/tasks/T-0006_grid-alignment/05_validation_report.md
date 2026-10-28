# Prüfung · 28.10.2026

## Lokale native Quellansicht

- Vorhandenes signiertes Mac-Testbundle `ece44ca` mit expliziter lokaler QML-Quelle gestartet; keine lokale Qt-Kompilation, kein Simulator. Erste Entwicklungsläufe zeigten eine schreibgeschützte Text-ImplicitHeight und einen ScrollView-Bindungszyklus; auf echte Zeilenhöhe und explizites Flickable umgestellt. Diese Fehlerläufe gelten nicht als Erfolg.
- Finaler Quellansichtslauf: Exit 0, 17 vorhandene QtTest-Klickschritte, beide Easy-Mode-Rundläufe mit Wiederherstellung, 13 Screenshots, 1060×680 und 0 erfasste QML-Laufzeitfehler. Dashboard, IR, Einstellungen und kompakte Ansicht selbst visuell angesehen.
- Zusätzliche temporäre lokale Ansichtsbeobachtung: Raster- und Inhaltsursprung vor Scrollen beide `[208,120]`, nach Änderung von `contentY` auf 123 beide `[208,-3]`. Alle beobachteten oberen Blockkanten und Blockhöhen sind Vielfache von 8. Easy-Mode-Feld relativ zur Inhaltsspalte `[808,424,280,40]`, ebenfalls vollständig im Grundmaß. Maschinenlesbar unter `evidence/source-scroll-geometry.json`. Die temporäre Beobachtungsdatei liegt nur im ignorierten Buildordner und wird nicht mitgeliefert.
- App-eigene Scrollansicht gespeichert und selbst angesehen; Scrollen wurde durch Änderung des echten Flickable-Inhaltsversatzes beobachtet, keine zusätzliche Mausgestenprüfung behauptet. Raster folgt demselben Inhalt; kein Gegeneinanderrutschen.
- Gerätewerte werden nur wie bisher angezeigt; kein IR-/NFC-Test, keine Helfer-/Firmwareinstallation. Geerbter Uhrabgleich bleibt unverändert. Keine neue Funktionsabnahme oder Veröffentlichung.

## Noch offen

Mac-/Linux-CI für diesen UI-Stand und tatsächlich gestartetes neues Standardbundle. Quellansicht ist noch kein Nachweis der gebündelten Ressourcen. Ergebnisse werden nach dem Paketbau ergänzt.
