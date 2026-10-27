# Prüfung der Planungsdokumentation

Datum: 26.10.2026.
Ausgangsrevision des gelesenen Forks: `1d26683ff5b751f219a4e40443b7c8355eb39469` auf Branch `dev`.
Arbeitsverzeichnis vor Dokumentation: sauber.

## Dokumentenprüfung

- Die Entscheidungen 1–15 in `01_scope.md` entsprechen den fünf beantworteten Fragerunden.
- Auswahl „alle drei Layouts“ als drei wählbare Modi erhalten, nicht auf eine Variante reduziert.
- Cloud-Anbieter und eigener Speicher gemeinsam erfasst; keine eigene gehostete uFlipper-Cloud als Auftrag interpretiert.
- Automatische dauerhafte Aufzeichnung und automatische Synchronisierung als zusammenhängende Anforderungen erfasst.
- Verschlüsselungswahl pro neuer Verbindung und zunächst deaktivierte automatische Bereinigung erhalten.
- Offene Machbarkeit und spätere Release-Abnahme ausdrücklich sichtbar.

## Bisherige technische Erkenntnisse: nur Quellprüfung

- qFlipper: C++-Backend und Qt/QML-Anwendung; vorhandene Dateiübertragung, Backups, Firmware-Verwaltung und Bildschirm-/Tasteneingabe.
- Momentum mntm-012: IR-CLI mit dekodiertem Empfang und RAW-Pulszeiten; RPC-Wege für gespeicherte IR-/Sub-GHz-Signale und NFC-Emulation; NFC-CLI unter anderem mit Scanner-, Dump- und Kartenkommunikationsfunktionen.
- Daraus folgt noch kein vollständiger Arbeitsablauf in uFlipper und kein Nachweis eines systemweiten NFC-Readers.

Quellen:

- [qFlipper-Struktur](https://github.com/flipperdevices/qFlipper#project-structure)
- [IR-Empfang in Momentum mntm-012](https://github.com/Next-Flip/Momentum-Firmware/blob/mntm-012/applications/main/infrared/infrared_cli.c)
- [IR-Empfänger und Demodulation](https://docs.flipper.net/zero/infrared/read)
- [NFC-CLI-Bereiche](https://github.com/Next-Flip/Momentum-Firmware/blob/mntm-012/applications/main/nfc/cli/nfc_cli_commands.c)
- [App-RPC](https://github.com/Next-Flip/Momentum-Firmware/blob/mntm-012/applications/services/rpc/rpc_app.c)

Keine Builds, Tests, Hardware- oder Cloud-Aktionen für diese Planungsdokumentation ausgeführt. Keine Runtime-Verifikation behauptet.

## Ergänzung 27.10.2026

Die Produktentscheidungen umfassen jetzt 1–16. Die zusätzliche Antwort erlaubt SD-Hilfsprogramme/Skripte nach ausdrücklicher Freigabe bei unveränderter Momentum-Firmware; sie aktiviert oder installiert nichts. Die frühere Prüfung oben bleibt als Stand vom 26.10. erhalten. Aktuelle Dokument- und Quellenprüfung: [T-0002 Prüfbericht](../T-0002_technical-plan/05_validation_report.md).

## Prüfung der Auswahlrunde · 27.10.2026

Aktuell sind Entscheidungen 1–22 dokumentiert. Die bisherigen Prüfabschnitte beschreiben frühere Planungsstände. Die sechs neuen Antworten wurden mit Scope, Betriebs-/Cloud-/UI-Abläufen und Abnahmeszenarien abgeglichen; CSV und Markdown bleiben inhaltlich gleich. Dokumentverweise und Quellen-IDs geprüft. Keine Laufzeitprüfung oder Anwendungscodeänderung, kein Commit/Push.
