# Prüfung · 28.10.2026

Abschlussstand: `c2eea85`, CI `40777350400` mit vier erfolgreichen Jobs; neues Standard-Mac-Bundle am echten Flipper nativ gestartet und angesehen. Rasterkorrektur abgeschlossen, isolierter Klick-Prüfbefund #1 weiterhin offen. Restlicher Ausbau bleibt verschoben. Historie und Grenzen unten getrennt.

## Lokale native Quellansicht

- Vorhandenes signiertes Mac-Testbundle `ece44ca` mit expliziter lokaler QML-Quelle gestartet; keine lokale Qt-Kompilation, kein Simulator. Erste Entwicklungsläufe zeigten eine schreibgeschützte Text-ImplicitHeight und einen ScrollView-Bindungszyklus; auf echte Zeilenhöhe und explizites Flickable umgestellt. Diese Fehlerläufe gelten nicht als Erfolg.
- Finaler Quellansichtslauf: Exit 0, 17 vorhandene QtTest-Klickschritte, beide Easy-Mode-Rundläufe mit Wiederherstellung, 13 Screenshots, 1060×680 und 0 erfasste QML-Laufzeitfehler. Dashboard, IR, Einstellungen und kompakte Ansicht selbst visuell angesehen.
- Zusätzliche temporäre lokale Ansichtsbeobachtung: Raster- und Inhaltsursprung vor Scrollen beide `[208,120]`, nach Änderung von `contentY` auf 123 beide `[208,-3]`. Alle beobachteten oberen Blockkanten und Blockhöhen sind Vielfache von 8. Easy-Mode-Feld relativ zur Inhaltsspalte `[808,424,280,40]`, ebenfalls vollständig im Grundmaß. Maschinenlesbar unter `evidence/source-scroll-geometry.json`. Die temporäre Beobachtungsdatei liegt nur im ignorierten Buildordner und wird nicht mitgeliefert.
- App-eigene Scrollansicht gespeichert und selbst angesehen; Scrollen wurde durch Änderung des echten Flickable-Inhaltsversatzes beobachtet, keine zusätzliche Mausgestenprüfung behauptet. Raster folgt demselben Inhalt; kein Gegeneinanderrutschen.
- Gerätewerte werden nur wie bisher angezeigt; kein IR-/NFC-Test, keine Helfer-/Firmwareinstallation. Geerbter Uhrabgleich bleibt unverändert. Keine neue Funktionsabnahme oder Veröffentlichung.

## Übergang zum Paketbau

Nach den Quellansichtsläufen waren Mac-/Linux-CI und ein neues gestartetes Standardbundle noch offen. Diese spätere Paketprüfung ist unten dokumentiert; die Quellansicht wird nicht nachträglich als Paketlauf bezeichnet.

## Zwischenfenster und Linux-CI

- Einzelner Lauf bei 1173×731 beendet sich beim Rückwechsel Bibliothek → Empfang mit `Tool tab click did not reach expected tab`; Fehlerbild zeigt 80 Pixel Inhaltsversatz. Kein QML-Fehler, aber dieser Klicklauf ist fehlgeschlagen. Der ursprüngliche Fehlerlauf bleibt erhalten.
- Ohne Änderung der Produkt-QML bestanden anschließend drei native Läufe mit zusätzlicher lokaler pressed/clicked-/Scrollbeobachtung: zweimal 1173×731 und einmal alle Seiten ab 1060×680. Je 17 Klickschritte, Einstellungsrundläufe, 13 Screenshots und 0 erfasste QML-Fehler; dabei kein Inhaltsversatz im Tabwechsel. Maschinenlesbar: `evidence/native-size-observations.json`. Produktursache oder externe Eingabeeinwirkung **nicht belegt**; kein angeblicher Fix durch bloße Wiederholung.
- Offener Untersuchungsbefund: [GitHub #1](https://github.com/JoKeks2023/uFlipper/issues/1). Issue-Suche findet keine Dublette; Repo-Issues sind inzwischen verfügbar. Connector-Schreibzugriff weiterhin HTTP 403, deshalb über bereits eingerichteten lokalen GitHub-Zugang angelegt. Keine Repo-Einstellungen geändert. Frühere Aussagen über deaktivierte Issues in T-0003 waren der damalige Stand.
- CI `40774081195`, Code `bace490`: beide Core-Jobs und Linux-Desktop vollständig erfolgreich. Linux-App nativ offscreen gestartet, reguläre 17 Klickschritte bestanden. Linux-Dashboard und Einstellungen selbst angesehen; Gerät offline, keine Linux-Hardwareabnahme. `evidence/linux-ui-bace490.json`.

## Mac-Baufehler und gezielte Korrektur

- CI `40774081195` scheitert im Universal-Mac-Bau beim RCC-Aufruf mit fehlender `build_mac/application/.qm/en_US.qm`; kein `lrelease`-Aufruf vorher im Joblog. Linux und beide Core-Jobs sind grün. Kein grünes Mac-Paket für diesen Lauf behauptet.
- Qt-6.8.3-Quellen geprüft: `lrelease.prf` erzeugt `QM_FILES`; bei eingebetteten Übersetzungen fehlt dort die `target_predeps`-Variante aus dem anderen Zweig. RCCs Ressourcenabhängigkeiten verwenden `remove_no_exist`. Befund ist eine fehlende explizite Erzeugungsabhängigkeit im parallelen Build, keine Raster-QML-Laufzeitstörung. Primärquellen: [Qt lrelease.prf](https://github.com/qt/qtbase/blob/v6.8.3/mkspecs/features/lrelease.prf), [Qt resources.prf](https://github.com/qt/qtbase/blob/v6.8.3/mkspecs/features/resources.prf).
- `lrelease`-Feature ausdrücklich laden und sämtliche erzeugten `QM_FILES` als RCC-Abhängigkeiten hinzufügen. Frühe CI-Prüfung fordert auf beiden Plattformen nur den Übersetzungs-Ressourcen-Target an und kontrolliert die tatsächlich erzeugte QM-Datei, bevor der große Build beginnt. Kein Löschen/Überspringen der Übersetzung und kein lokaler Qt-Komplettbau. Neue Ausführung noch erforderlich.

- Korrektur `c2eea8535fc35ab6c1a57808dda91036a8616e12`, neuer CI-Lauf `40777350400`: frühe Erzeugung des Übersetzungs-Ressourcen-Targets mit vier Jobs und Prüfung der echten QM-Datei auf Mac und Linux erfolgreich. Linux-Desktop und beide Core-Jobs ebenfalls erfolgreich. Vollständiger Universal-Mac-Bau noch in Arbeit; noch kein neues gestartetes Paket behauptet. Build-Bug unter [#2](https://github.com/JoKeks2023/uFlipper/issues/2), Status In Progress im Issue; ohne PR/Merge nicht geschlossen.

## Abschließende Paketprüfung

- [CI 40777350400](https://github.com/JoKeks2023/uFlipper/actions/runs/40777350400), Code `c2eea8535fc35ab6c1a57808dda91036a8616e12`: vier Jobs erfolgreich, einschließlich Qt-Verträgen, früher Übersetzungs-Ressourcenprüfung, vollständigem Universal-Paket und eigenständigem Mac-Paketstart bei verborgenem Entwickler-Qt. Vollständiger Linux-UI-Beleg für den Fixstand: `evidence/linux-ui-c2eea85.json`; zwei unveränderte Offline-Screenshots unter `evidence/screenshots/`.
- Lognachweis der Buildkorrektur: zuerst echte `lrelease`-Erzeugung der QM-Datei, danach RCC — sowohl im isolierten frühen Target als auch im vollständigen Mac-Bau. `evidence/translation-order-c2eea85.json`. Build-Bug #2 im Featurebranch praktisch behoben, Issue bis zu einem späteren PR/Merge offen; keine Übersetzung deaktiviert.
- Mac-Artefakt `12856720558` heruntergeladen, SHA-256 `6806504227b31be1b38dcf548eeae66d44e20cf94931fdb31789a954600475d0` geprüft; eigenes vorheriges Testbundle erhalten. Strikte vollständige Signatur erfolgreich, `x86_64 arm64`, Bundle-ID `com.jorisconrad.flipperzeroutil`.
- **Neues Standardbundle ohne `--ui-source`** auf Apple Silicon / macOS 27.0.1 am real verbundenen Momentum-Flipper gestartet. Lauf endet `2026-10-28T22:22:15.009Z`, Exit 0: 17 reale QtTest-Klickschritte, beide Easy-Mode-Rundläufe mit Wiederherstellung, 13 Screenshots, 1060×680, 0 erfasste QML-Laufzeitfehler. Kein neuer Warnungs-/Fehlerhinweis im beobachteten Laufzeitlog. `evidence/mac-bundle-ui-c2eea85.json`.
- Dashboard, IR, Einstellungen und kompakte Dashboardansicht aus diesem Bundle selbst visuell angesehen: Raster/Abstände entsprechen dem geprüften Quellstand; reale Gerätewerte und korrigierte Momentum-Speicherzuordnung sichtbar. Native Bilder lokal unter `build_mac/ui-review-grid-bundle-c2eea85/` und im Chat-Artefaktordner, Hashes im JSON. Linux-Bilder enthalten kein verbundenes Gerät.
- Diese erfolgreichen Läufe klären den früheren einzelnen Zwischenfenster-Klickfehler #1 nicht nachträglich. Ursachenklärung bleibt offen; keine Behauptung allgemeiner Fehlerfreiheit. Keine weiteren Gerätetests, IR-/NFC-Sitzungen oder Helferinstallationen gestartet. Physisches Abziehen/Wiederverbinden, Linux-Hardware und gesamte Produktabnahme gehören nicht zu diesem erledigten Darstellungsbauschritt.

- Nach den Prüfungen normale aktualisierte Mac-App ohne Testparameter wieder geöffnet; laufender Prozess bestätigt.
