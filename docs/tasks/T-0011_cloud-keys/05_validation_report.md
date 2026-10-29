# Validierung

Lokaler kleiner C++-Build unter macOS/arm64: 4/4 CTest-Ziele erfolgreich am 29.10.2026, einschließlich neuem `key_envelope_contract` (ca. 0,63 s).

Beobachteter Host-Dateipfad: Datei mit zufälligem Datenschlüssel verschlüsselt; denselben Schlüssel unabhängig per Testpasswort und zufälligem Recovery-Key entsperrt; beide Wege entschlüsseln die unveränderte Chiffre zu identischen SHA-256-Werten. Passwort neu verpackt und identischen Datenschlüssel erhalten. Keine echten Nutzerdaten/Cloud-Zugänge verwendet.

Zurückgewiesen: falsches Passwort, alter Testpasswortwert am neuen Umschlag, falscher Recovery-Key, falsche Daten-ID/Methode, veränderte Nonce/ID/Chiffre, unbekannter Algorithmus/Version, zu große CPU-/Speicherwerte, widersprüchliche Recovery-KDF-Felder, fehlende/zusätzliche Bytes und leeres Passwort. Nach Schlüssel-Move ist der alte Speicher nullgesetzt.

CI [40934565445](https://github.com/JoKeks2023/uFlipper/actions/runs/40934565445), Quelle f84242a: Kernprüfungen Mac und Linux jeweils 4/4 erfolgreich; neues key_envelope_contract auf Mac ca. 0,62 s, auf Linux ca. 0,20 s. Kein Cloud-, Betriebssystem-Schlüsselspeicher- oder Zwei-Computer-Nachweis.

Unabhängig mit Python/FFI gegen das echte libsodium geprüft: vom C++-Kern erzeugte 133-Byte-Umschläge strukturell gelesen, gespeicherte Big-Endian-KDF-Parameter ausgewertet, beide Methoden erfolgreich zu denselben ausschließlich synthetischen Testschlüsseln entschlüsselt; veränderte authentifizierte Metadaten zurückgewiesen. [Ergebnis](evidence/independent-envelope.json). Keine Testgeheimnisse ins Repo aufgenommen.
