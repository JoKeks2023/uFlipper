# Prüfung · 28.10.2026

## Lokale native Quellansicht

- Vorhandenes signiertes Mac-Testbundle `ece44ca` mit expliziter lokaler QML-Quelle gestartet; keine lokale Qt-Kompilation, kein Simulator. Erste Entwicklungsläufe zeigten eine schreibgeschützte Text-ImplicitHeight und einen ScrollView-Bindungszyklus; auf echte Zeilenhöhe und explizites Flickable umgestellt. Diese Fehlerläufe gelten nicht als Erfolg.
- Finaler Quellansichtslauf: Exit 0, 17 vorhandene QtTest-Klickschritte, beide Easy-Mode-Rundläufe mit Wiederherstellung, 13 Screenshots, 1060×680 und 0 erfasste QML-Laufzeitfehler. Dashboard, IR, Einstellungen und kompakte Ansicht selbst visuell angesehen.
- Zusätzliche temporäre lokale Ansichtsbeobachtung: Raster- und Inhaltsursprung vor Scrollen beide `[208,120]`, nach Änderung von `contentY` auf 123 beide `[208,-3]`. Alle beobachteten oberen Blockkanten und Blockhöhen sind Vielfache von 8. Easy-Mode-Feld relativ zur Inhaltsspalte `[808,424,280,40]`, ebenfalls vollständig im Grundmaß. Maschinenlesbar unter `evidence/source-scroll-geometry.json`. Die temporäre Beobachtungsdatei liegt nur im ignorierten Buildordner und wird nicht mitgeliefert.
- App-eigene Scrollansicht gespeichert und selbst angesehen; Scrollen wurde durch Änderung des echten Flickable-Inhaltsversatzes beobachtet, keine zusätzliche Mausgestenprüfung behauptet. Raster folgt demselben Inhalt; kein Gegeneinanderrutschen.
- Gerätewerte werden nur wie bisher angezeigt; kein IR-/NFC-Test, keine Helfer-/Firmwareinstallation. Geerbter Uhrabgleich bleibt unverändert. Keine neue Funktionsabnahme oder Veröffentlichung.

## Noch offen

Mac-/Linux-CI für diesen UI-Stand und tatsächlich gestartetes neues Standardbundle. Quellansicht ist noch kein Nachweis der gebündelten Ressourcen. Ergebnisse werden nach dem Paketbau ergänzt.

## Zwischenfenster und Linux-CI

- Einzelner Lauf bei 1173×731 beendet sich beim Rückwechsel Bibliothek → Empfang mit `Tool tab click did not reach expected tab`; Fehlerbild zeigt 80 Pixel Inhaltsversatz. Kein QML-Fehler, aber dieser Klicklauf ist fehlgeschlagen. Der ursprüngliche Fehlerlauf bleibt erhalten.
- Ohne Änderung der Produkt-QML bestanden anschließend drei native Läufe mit zusätzlicher lokaler pressed/clicked-/Scrollbeobachtung: zweimal 1173×731 und einmal alle Seiten ab 1060×680. Je 17 Klickschritte, Einstellungsrundläufe, 13 Screenshots und 0 erfasste QML-Fehler; dabei kein Inhaltsversatz im Tabwechsel. Maschinenlesbar: `evidence/native-size-observations.json`. Produktursache oder externe Eingabeeinwirkung **nicht belegt**; kein angeblicher Fix durch bloße Wiederholung.
- Offener Untersuchungsbefund: [GitHub #1](https://github.com/JoKeks2023/uFlipper/issues/1). Issue-Suche findet keine Dublette; Repo-Issues sind inzwischen verfügbar. Connector-Schreibzugriff weiterhin HTTP 403, deshalb über bereits eingerichteten lokalen GitHub-Zugang angelegt. Keine Repo-Einstellungen geändert. Frühere Aussagen über deaktivierte Issues in T-0003 waren der damalige Stand.
- CI `40774081195`, Code `bace490`: beide Core-Jobs und Linux-Desktop vollständig erfolgreich. Linux-App nativ offscreen gestartet, reguläre 17 Klickschritte bestanden. Linux-Dashboard und Einstellungen selbst angesehen; Gerät offline, keine Linux-Hardwareabnahme. `evidence/linux-ui-bace490.json`.

## Mac-Baufehler und gezielte Korrektur

- CI `40774081195` scheitert im Universal-Mac-Bau beim RCC-Aufruf mit fehlender `build_mac/application/.qm/en_US.qm`; kein `lrelease`-Aufruf vorher im Joblog. Linux und beide Core-Jobs sind grün. Kein grünes Mac-Paket für diesen Lauf behauptet.
- Qt-6.8.3-Quellen geprüft: `lrelease.prf` erzeugt `QM_FILES`; bei eingebetteten Übersetzungen fehlt dort die `target_predeps`-Variante aus dem anderen Zweig. RCCs Ressourcenabhängigkeiten verwenden `remove_no_exist`. Befund ist eine fehlende explizite Erzeugungsabhängigkeit im parallelen Build, keine Raster-QML-Laufzeitstörung. Primärquellen: https://github.com/qt/qtbase/blob/v6.8.3/mkspecs/features/lrelease.prf und https://github.com/qt/qtbase/blob/v6.8.3/mkspecs/features/resources.prf .
- `lrelease`-Feature ausdrücklich laden und sämtliche erzeugten `QM_FILES` als RCC-Abhängigkeiten hinzufügen. Frühe CI-Prüfung fordert auf beiden Plattformen nur den Übersetzungs-Ressourcen-Target an und kontrolliert die tatsächlich erzeugte QM-Datei, bevor der große Build beginnt. Kein Löschen/Überspringen der Übersetzung und kein lokaler Qt-Komplettbau. Neue Ausführung noch erforderlich.
