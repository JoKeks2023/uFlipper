# Umsetzung

Implementiert; Quellansicht nativ geprüft, gebündeltes Paket noch in Prüfung.

| Before | After | Why |
| --- | --- | --- |
| Festes 40-Pixel-Raster unter dem gesamten Arbeitsbereich | Inhaltsbezogenes Raster mit gemeinsamem 8-Pixel-Grundmaß | Karten, Zeilen und Raster teilen eine Geometrie |
| Raster bleibt beim Scrollen stehen | Raster liegt im selben Scrollinhalt | Ausrichtung bleibt beim Scrollen erhalten |
| Abstände und Höhen unabhängig voneinander | Gemeinsame Maße und auf volle Einheiten gerundete Textzeilen | Kein Versatz durch Textumbruch oder Fenstergröße |

- `WorkbenchGeometry.js` liefert Grundmaß, Außenrand, Zwischenräume, Steuerelementhöhe und Detailspalte. Textzeilen passen ins Grundmaß; Felder sitzen an der Unterkante ihrer Zeilen, damit keine halben Einheiten entstehen.
- Raster in den eigenen Flickable-Inhalt verschoben; feine Hilfslinien zurückhaltend, Hauptlinien weiterhin im qFlipper-Stil. Gleicher Ursprung und Maßstab in beiden Koordinatenräumen.
- Datei-/Bildschirmbereich und IR-Diagramm verwenden passende Kanten und Maße. Geräteverhalten unverändert, übriger Ausbau verschoben.
