# Umsetzung

GitHub [#6](https://github.com/JoKeks2023/uFlipper/issues/6) und [#7](https://github.com/JoKeks2023/uFlipper/issues/7), In Progress. Huly-Tools in dieser Sitzung nicht verfügbar.

- Type-4-NDEF-Mapping-2-Leseautomat: NDEF-Anwendung, Capability Container, dynamische Datei-ID und Längen, begrenzte READ-BINARY-Blöcke und abschließender Längenvergleich. Maximal 32.765 Bytes; Parser weist ungültige/gekürzte/chunked Datensätze ausdrücklich ab. Schreibkommandos werden im NDEF-Modus nicht erzeugt.
- NDEF-Modus auf der bestehenden NFC-Seite: Text-/URI-Datensätze und begrenzte Rohdatenvorschau. Ungekürzte Kommandos und Antworten bleiben in Originalaufnahmen; Wiederöffnen erhält Zeitpunkte. Unterbrochene NDEF-Leseversuche bleiben trotz Kartenfehler als integritätsgeprüfte Originalaufnahmen lesbar.
- Experimentelle lokale VICC-Bridge in umgekehrter Verbindungsrichtung: nur 127.0.0.1:35963, nur während explizit gestarteter Reader-Sitzung. Verwendet NfcSession/NfcController und damit vorhandenen RPC-Besitzer, Ressourcenbelegung, Aufnahme und bestätigten Stop. Keine weitere serielle Verbindung.
- Reader-Kommandos/Antworten: kurze APDUs innerhalb der Helfer-/Kartengrenze unverändert weiterreichen; 32 letzte Antworten im RAM, Originalaufnahme vollständig. Power/Reset über Deaktivierung und erneute Aktivierung. Eigener T=1-Transport-ATR klar als Proxy-Information dokumentiert.
- Separater Universal-Testbuild des macOS-Reader-Bundles aus gepinnten, hashgeprüften virtualsmartcard-/PCSC-Quellen. GPL-Lizenz und Upstream-Hinweise erhalten. Lokales Protokoll, Größen- und Zeitgrenzen, korrekte Behandlung partieller/geschlossener Sockets und Mutex-Initialisierung ergänzt. Keine Registrierung durch den Builder.
- Native Klickprüfung um NDEF-/Reader-Moduswahl erweitert; Qt-Sitzungsverträge laufen vor den vollständigen CI-Paketbuilds.

Stand `02d9fbb`: alle fünf CI-Jobs erfolgreich. Neues Mac-Paket tatsächlich am angeschlossenen Flipper geprüft: NDEF- und Reader-Modus jeweils 9 Klicks, Start/Suche/Stop-Konflikt/Dateiansichtswechsel/erneutes Öffnen erfolgreich, 0 QML-Warnungen, Originalaufnahme geprüft. Keine Karte im Durchlauf und keine APDU versandt. Geprüftes Paket nach `build_mac/uFlipper.app` übernommen; vorheriger Build unter `build_mac/previous-be2a115` erhalten.

Offen: tatsächlicher OS-Slot/OpenSC-Zugriff und physische NDEF-/abhängige Kartenkommandos. Nutzer besitzt weiterhin keine passende Testkarte. Helfer 0.1.0 meldet eine im Leerlauf entfernte Karte nicht proaktiv; vollständige Reader-Lifecycle-Abnahme ist damit noch nicht belegt. Keine neue Helferversion installiert und keine Firmwareänderung.
