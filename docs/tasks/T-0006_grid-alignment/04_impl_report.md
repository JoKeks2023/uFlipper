# Umsetzung

Implementiert; Quellansicht und neu gebündeltes Standardpaket nativ geprüft.

| Before | After | Why |
| --- | --- | --- |
| Festes 40-Pixel-Raster unter dem gesamten Arbeitsbereich | Inhaltsbezogenes Raster mit gemeinsamem 8-Pixel-Grundmaß | Karten, Zeilen und Raster teilen eine Geometrie |
| Raster bleibt beim Scrollen stehen | Raster liegt im selben Scrollinhalt | Ausrichtung bleibt beim Scrollen erhalten |
| Abstände und Höhen unabhängig voneinander | Gemeinsame Maße und auf volle Einheiten gerundete Textzeilen | Kein Versatz durch Textumbruch oder Fenstergröße |

- `WorkbenchGeometry.js` liefert Grundmaß, Außenrand, Zwischenräume, Steuerelementhöhe und Detailspalte. Textzeilen passen ins Grundmaß; Felder sitzen an der Unterkante ihrer Zeilen, damit keine halben Einheiten entstehen.
- Raster in den eigenen Flickable-Inhalt verschoben; feine Hilfslinien zurückhaltend, Hauptlinien weiterhin im qFlipper-Stil. Gleicher Ursprung und Maßstab in beiden Koordinatenräumen.
- Datei-/Bildschirmbereich und IR-Diagramm verwenden passende Kanten und Maße. Geräteverhalten unverändert, übriger Ausbau verschoben.

- Notwendigen Build-Fix für fehlende Übersetzungsabhängigkeit ergänzt (`c2eea85`, [#2](https://github.com/JoKeks2023/uFlipper/issues/2)); reguläres UI-Verhalten bleibt unverändert. Die neue frühe Ressourcenprüfung ist auf beiden CI-Plattformen bestanden; kompletter Mac-Bau und nativer Standard-Paketstart sind inzwischen erfolgreich.
- Isolierten nativen Klick-Prüfbefund unter [#1](https://github.com/JoKeks2023/uFlipper/issues/1) offen erhalten, ohne durch Wiederholungen einen Fix zu behaupten.

Abschluss: Code `c2eea85`, CI `40777350400` mit vier grünen Jobs. Fertiges Universal-Bundle auf Apple Silicon tatsächlich gestartet und visuell geprüft; 17 Klickschritte und 13 Screenshots. Restlicher Produktausbau bleibt auf Nutzerwunsch verschoben, kein öffentlicher Release. Offener Prüf-Untersuchungsbefund #1 bleibt separat bestehen.
