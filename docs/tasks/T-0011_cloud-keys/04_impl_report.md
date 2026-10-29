# Umsetzung

GitHub [#11](https://github.com/JoKeks2023/uFlipper/issues/11), In Progress. Isolierter C++-Kern; keine Produktoberfläche/Cloud-Anbindung.

- Zufälliger 256-Bit-Datenschlüssel mit separater 128-Bit-ID. Passwort und zufälliger Wiederherstellungsschlüssel schützen denselben Datenschlüssel.
- `KeyEnvelope`: versionierter binärer Umschlag; XChaCha20-Poly1305 authentifiziert Schlüsselinhalt und vollständigen Header. Neue zufällige Nonce pro Umschlag; bei Passwort zusätzlich zufälliges Salt.
- Argon2id13 mit gespeicherten tatsächlichen Parametern: beim Erstellen 2 Operationen/64 MiB. Lesen akzeptiert nur Argon2id13, 2–6 Operationen und 64–256 MiB; unbekannte/überhöhte Werte werden vor der Ableitung zurückgewiesen.
- Daten-ID, Methode, Version, Parameter, Salt und Nonce sind an den authentifizierten Inhalt gebunden. Falscher erwarteter Daten-ID oder Entsperrmethode erzeugt keinen Datenschlüssel.
- Entsperrte Schlüssel haben einen nicht kopierbaren, beim Freigeben löschenden Besitzer. Temporäre abgeleitete und entschlüsselte Schlüssel werden auch bei Fehlern gelöscht. Aufrufende Schichten verantworten Passwort- und Quellschlüssellebensdauer.
- Passwortwechsel kann denselben Datenschlüssel neu verpacken. Zurückbehaltene alte Umschläge bleiben mit dem alten Geheimnis nutzbar; tatsächlicher Widerruf erfordert Datenschlüsselwechsel und Neuverschlüsselung. Wiederherstellungsschlüssel nicht automatisch entfernen oder exportieren.

Offen: sichere Schlüsseldatei-/Code-Bedienung, OS-Schlüsselspeicher für Anbieterzugänge, Cloud-Schlüsselkatalog/Revisions- und Konfliktregeln, Produktoberfläche und reale Anbieterintegration. Keine automatische Aktivierung für Nutzerdaten.

Primärquellen: [libsodium Argon2id/KDF](https://doc.libsodium.org/password_hashing/default_phf), [libsodium XChaCha20-Poly1305](https://doc.libsodium.org/secret-key_cryptography/aead/chacha20-poly1305/xchacha20-poly1305_construction).
