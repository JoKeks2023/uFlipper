# Prüfkriterien

- [ ] App nutzt bestehenden RPC-Besitzer; keine zweite serielle Verbindung.
- [ ] Keine NFC-Aktivierung ohne vorher erstellte lokale Aufnahme.
- [ ] Start/Antwort/Stop über Oberfläche; Empfangsdaten und Grenzen sichtbar.
- [ ] Stop, Timeouts und Verbindungsverlust unterscheiden sich; alte Antworten ändern keine neue Sitzung.
- [ ] Navigation in konkurrierende Geräteansichten verlangt Bestätigung und wartet auf Stop.
- [ ] Gespeicherte Sitzung wieder geöffnet, Integrität geprüft.
- [ ] Kern-/Qt-Vertragstests und Mac-/Linux-Builds erfolgreich.
- [ ] Neues natives Mac-Bundle tatsächlich gestartet und betroffene UI geklickt.
- [ ] Gerätetest aus der App mit Nutzer-iPhone beobachtet; ohne diesen bleibt Kartenpfad unvollständig verifiziert.
