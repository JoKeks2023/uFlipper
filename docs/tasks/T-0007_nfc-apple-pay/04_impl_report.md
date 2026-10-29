# Umsetzung

Vorhandenes kleines C++-Prüfprogramm erhält den expliziten Modus `--apple-pay`. Standard bleibt die bisherige Helfer-Lifecycle-Prüfung ohne NFC-Feld. Der neue Modus aktiviert für höchstens ein begrenztes Erkennungsfenster, sendet zweimal denselben festgelegten PPSE-SELECT und stoppt. Rohdaten verbleiben in der lokalen Aufnahme. Keine Produktoberfläche geändert.

Fehlerausgänge behalten den konkreten Fehler, markieren die Aufnahme als unterbrochen und protokollieren eine Lücke für den nicht aufgezeichneten Aufräumtransport. Stop-/RPC-Bestätigung bleibt unabhängig vom APDU-Erfolg sichtbar. Bisheriger Lifecycle-Modus ohne Feld am Gerät erneut bestätigt.
