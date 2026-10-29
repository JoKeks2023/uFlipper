# Prüfkriterien

- [x] App nutzt bestehenden RPC-Besitzer; keine zweite serielle Verbindung.
- [x] Keine NFC-Aktivierung ohne vorher erstellte lokale Aufnahme.
- [x] Start/Antwort/Stop über Oberfläche; Empfangsdaten und Grenzen sichtbar.
- [x] Stop, Timeouts und Verbindungsverlust unterscheiden sich; alte Antworten ändern keine neue Sitzung.
- [x] Navigation in konkurrierende Geräteansichten verlangt Bestätigung und wartet auf Stop.
- [x] Gespeicherte Sitzung wieder geöffnet, Integrität geprüft.
- [x] Kern-/Qt-Vertragstests und Mac-/Linux-Builds erfolgreich.
- [x] Neues natives Mac-Bundle tatsächlich gestartet und betroffene UI geklickt.
- [x] Gerätetest aus der App mit Nutzer-iPhone beobachtet; ohne diesen bleibt Kartenpfad unvollständig verifiziert.

Gilt ausschließlich für den unter 01_scope beschriebenen PPSE-Arbeitsablauf. Fehlerfälle mit Zeitüberschreitung, Datenträgerfehlern und Verbindungsverlust sind in Qt-Vertragstests belegt; kein vollständiger physischer Störungstest. Einzelheiten unter 05_validation_report.
