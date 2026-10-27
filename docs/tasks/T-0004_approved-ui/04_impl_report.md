# Umsetzung

In Arbeit. Freigabe der drei Entwürfe aus T-0003 liegt vor.

- Seitenleiste und resizierbare Arbeitsfläche, Echtwerte-Dashboard inklusive Speichergrößen, IR-/Werkzeugansicht, fünf Settings-Bereiche implementiert. Datei- und Bildschirmfunktionen eingebunden. Nicht angeschlossene Funktionen explizit gekennzeichnet.
- Aufnahme-/Backupkataloge werden lesend abgefragt; Testsicherungen als solche ausgewiesen. Easy Mode speichert die echte Einstellung. Navigation/Schließen während Dateiübertragung gesperrt.
- Logger leitet Nachrichten aus dem Renderthread in seinen Besitzerthread; gezielter Qt-Vertragstest ergänzt. Ein alter QML-Deklarationshinweis korrigiert.
