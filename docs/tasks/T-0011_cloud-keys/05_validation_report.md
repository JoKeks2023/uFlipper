# Validierung

Lokaler kleiner C++-Build unter macOS/arm64: 4/4 CTest-Ziele erfolgreich am 29.10.2026, einschließlich neuem `key_envelope_contract` (ca. 0,63 s).

Beobachteter Host-Dateipfad: Datei mit zufälligem Datenschlüssel verschlüsselt; denselben Schlüssel unabhängig per Testpasswort und zufälligem Recovery-Key entsperrt; beide Wege entschlüsseln die unveränderte Chiffre zu identischen SHA-256-Werten. Passwort neu verpackt und identischen Datenschlüssel erhalten. Keine echten Nutzerdaten/Cloud-Zugänge verwendet.

Zurückgewiesen: falsches Passwort, alter Testpasswortwert am neuen Umschlag, falscher Recovery-Key, falsche Daten-ID/Methode, veränderte Nonce/ID/Chiffre, unbekannter Algorithmus/Version, zu große CPU-/Speicherwerte, widersprüchliche Recovery-KDF-Felder, fehlende/zusätzliche Bytes und leeres Passwort. Nach Schlüssel-Move ist der alte Speicher nullgesetzt.

Mac-/Linux-CI für dieses neue Kernmodul noch offen. Kein Cloud-, Betriebssystem-Schlüsselspeicher- oder Zwei-Computer-Nachweis.
