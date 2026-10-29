# Validierung

## Aktueller Stand

Code: `02d9fbb303d660934d3b802687e88d9784495f07`. [CI 40884130592](https://github.com/JoKeks2023/uFlipper/actions/runs/40884130592) **alle fünf Jobs erfolgreich**: Linux-Desktop, Mac-Universal-Paket, Kernprüfungen beider Plattformen und Reader-Komponente. Qt-NFC-Verträge vor dem Paketbau auf beiden Plattformen grün (einschließlich bisheriger PPSE-Verträge).

## Lokale beobachtete Prüfungen

- Kleiner portabler C++-Build, kein Qt-Großbuild: CMake/CTest **3/3 grün** (Kern, serieller Vertrag, NDEF). [JUnit](evidence/core-macos-local.xml)
- Zustandsabhängige NDEF-Karten-Fixur fordert korrekte Anwendungs-/Dateiauswahl; prüft kleine und große Datensätze, geringe Framegröße, leere NDEF-Datei, gekürzte Datensätze/CC, falsche Mapping-Version, gesperrte Datei und geänderte Länge. Das ist ein Software-Vertrag, kein physischer Kartenbeleg.
- Mac-Reader-Bundle als Universal x86_64/arm64 mit Minimum 13.0 gebaut; strenge Codesign-Prüfung erfolgreich. Finaler Binär-SHA-256 `f7d21ecee82ba42a5322c802e16d92ea3e2c4b40db245aae9917d9a7e6fbff13`. [Manifest](evidence/reader-local-manifest.json)
- Reader-IFD-Bibliothek direkt und ohne Systemregistrierung geladen: lokale synthetische Gegenstelle, fragmentierte Rahmen, ATR, unveränderte APDU-/Statusweitergabe, Power/Reset, zu große Kommandos und fehlende Karte geprüft. **Kein OS-Reader-Slot und kein unabhängiger PC/SC-Clientnachweis.** [Ergebnis](evidence/reader-local-contract-result.json)

## Noch offen

Experimentelle Systemkomponente wurde zur ausdrücklichen Installationsfreigabe vorgelegt; noch nicht registriert. NDEF-Karte fehlt laut erneuter Nutzerantwort. Kein vollständiger NDEF-/Systemreader-Abschluss und kein Release.

## Native App am realen Mac/Flipper

- CI-Artefakt 12901954167, Archiv SHA-256 `4f7dd90c7cb2f0f4dcbcfd494a257e7e3a2da996406f7101b8bdca16d96387dd`; Universal x86_64/arm64, Bundle-ID korrekt, strenge Codesign-Prüfung erfolgreich. Binärdatei `b61701d8dc9da0f12343b2f6cd247c198ea6bb583385bb096730b78e42d2e364`.
- NDEF-Modus, 13:59:53 UTC: 9 echte UI-Klicks einschließlich Modus-Dropdown, Start/Suche, „Weiterlesen“, bestätigtem Stop vor Dateiansichtswechsel und Wiederöffnen. 0 QML-Warnungen. Aufnahme `e5f406f74eb5ca0af1af60efb9f608a1`.
- Reader-Modus, 14:00:33 UTC: derselbe native Ablauf, ebenfalls 9 Klicks und 0 QML-Warnungen. Aufnahme `cd3eaff1ea4455f9d36a9780542f3773`. Dies prüft ausschließlich den Reader-Modus innerhalb der App, keinen OS-Slot.
- Beide Originalaufnahmen unabhängig gegen Manifest-SHA-256 und Bytezahl geprüft; bestätigtes Helper-Closed aufgezeichnet. In beiden Durchläufen keine Karte und keine APDU. [Native Evidenz](evidence/mac-native-02d9fbb.json)
- Linux tatsächlich gestartet und navigiert: 18 Klicks, 14 Screenshots, 0 erfasste QML-Fehler. [Evidenz](evidence/linux-ui-02d9fbb.json)
- Geprüfter Mac-Stand lokal geöffnet; vorheriges Paket erhalten. Firmware und freigegebener Helfer unverändert.

## Unabhängiger Client vorbereitet

Offizieller [OpenSC-Client 0.27.1](https://github.com/OpenSC/OpenSC/releases/tag/0.27.1) temporär entpackt; verifiziertes, von Apple notarisiertes Originalpaket, keine Installer-Skripte ausgeführt. Für den Betrieb ohne globale Installation nur Bibliotheks-Ladepfad der kopierten Binärdatei angepasst und diese Kopie ad-hoc neu signiert; OpenSC-Logik unverändert. `--version` und `--list-readers` tatsächlich aufgerufen: Version 0.27.1, derzeit **No smart card readers found**. Das ist der Ausgangsbefund vor Reader-Installation. [Evidenz](evidence/opensc-baseline.json)

## Quellen und technische Grenzen

[NFC Forum Type 4](https://nfc-forum.org/build/specifications/type-4-tag-specification) beschreibt NDEF über ISO-DEP. Dieser Ausbau unterstützt zunächst Type 4A/Mapping 2; Type 2/NTAG, Mapping 3, APDU-/NDEF-Chaining und weitere Tagtypen sind damit nicht umgesetzt. [virtualsmartcard-macOS-Ansatz](https://github.com/frankmorgner/vsmartcard/blob/93286a90d2411120ae17299f9902e1fdd8954185/virtualsmartcard/doc/README.txt.in) muss auf dem tatsächlichen Ziel-Mac beobachtet werden; historische Plattformhinweise ersetzen den Nachweis nicht.
