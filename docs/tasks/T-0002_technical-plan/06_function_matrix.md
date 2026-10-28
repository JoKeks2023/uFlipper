# Funktionsmatrix · Quelle ist noch kein Laufzeitnachweis

Stand: 28.10.2026. Firmware und Fork sind in [10_sources.md](10_sources.md) festgelegt. Keine Zeile ist vollständig abgenommen. Erste ausdrücklich begrenzte Teilnachweise stehen im Laufzeitstatus der CSV sowie unter [T-0003](../T-0003_foundation/05_validation_report.md) und [T-0004](../T-0004_approved-ui/05_validation_report.md). Status bezeichnet den Weg im Quellcode oder Planungsbedarf, nicht eine fertige uFlipper-Funktion.

| Kürzel | Bedeutung |
| --- | --- |
| C | Schnittstelle im geprüften Quellstand belegt. Desktop-Integration und Laufzeitnachweis fehlen ganz oder teilweise. |
| H | Eigener Helfer ist ein Kandidat. Nur nach ausdrücklicher Installationsfreigabe; API/ABI und Verhalten erst beweisen. |
| P | Gewünschte Host-Funktion, für uFlipper noch zu entwerfen/implementieren. |
| O | Integrationsweg oder konkreter Umfang offen; keine Machbarkeitszusage. |
| L | Eine so benannte Messgröße ist über den eingebauten geprüften Datenweg nicht verfügbar. Die geeignete Alternative bleibt im Werkzeugbereich. |

Die Spalte „Abnahme“ verweist auf [09_milestones_acceptance.md](09_milestones_acceptance.md). `U01` bezeichnet die bestätigten Nutzerentscheidungen in [T-0001](../T-0001_product-plan/01_scope.md), keine technische Quelle. Alle anderen Belege stehen in [10_sources.md](10_sources.md).

Die [filterbare CSV](function_matrix.csv) enthält zusätzlich Ressourcen, Datenprofile und den expliziten Laufzeitstatus je Zeile. Diese Matrix inventarisiert die Gerätebereiche und wesentlichen Abläufe; sie ist kein vollständiger Katalog aller FAPs, Kartenprotokolle oder externen Module. Neue Anbindungen brauchen eigene Zeilen/Nachweise, ohne den vereinbarten Zielumfang stillschweigend zu verkleinern.

## Gemeinsame Daten- und Stoppverträge

Die Profile sind Architekturvorschläge für die Umsetzung. „LIVE“ bezeichnet nur neue, vom Gerät tatsächlich gelieferte Daten. Kein Profil verspricht eine bisher ungemessene Rate.

| Profil | Daten, Einheit und Zeitbezug | Abschluss, Verlust und Fehler |
| --- | --- | --- |
| STATE | Versionierte Schlüssel/Werte, Originaleinheit plus normalisierte Einheit, Host-Empfangszeit und Alter; Gerätezeit nur bei echter Quelle. | Polling anhalten; nicht mehr aktualisierte Werte als veraltet/fehlend zeigen. |
| FRAME | Bildschirmbitmap, Host-Empfangszeit, Bildfolge. | RPC-Stream stoppen; fehlende Frames sind keine Messlücke anderer Module. |
| IR | Dekodierte Frames mit Protokoll/Adresse/Befehl/Repeat; Empfangszeit am Host ist nicht der Beginn des optischen Signals. | CLI Ctrl+C bis bestätigter Rückkehr; unvollständiges letztes Frame kennzeichnen. |
| PULSE | Pegel/Vorzeichen und Dauer in der Quellenzeiteinheit, i.d.R. µs; Geräteticks nur wenn geliefert. Host-Zeit und rekonstruierte relative Zeit getrennt. | Reset-, Overrun- und Parserlücken erhalten; keinen lückenlosen absoluten Zeitstrahl erfinden. |
| PACKET | Frequenz in Hz, Modul/Preset, Protokollfelder und Originalausgabe; Host-Empfangszeit. | Ctrl+C/Handler-Stop bestätigen, Fehler und unbekannte Pakete erhalten. |
| RSSI | Frequenz Hz, Pegel dBm nur bei entsprechender Quelle, Abtast-/Einschwingzeit nach Messung; Sweep nacheinander. | Stop und Radio-Idle bestätigen; fehlende Frequenzpunkte sichtbar. |
| CARD | Protokoll, UID/Schlüssel/Kartendaten je Typ, vollständig/teilweise/gesperrt; Transaktionsfolge statt erfundener Abtastrate. | Abbruch und Kartenverlust explizit; keine unvollständigen Dumps als vollständig deklarieren. |
| APDU | Kommando/Antwort/Status, Sequenz und Host-Zeiten; Karten-/Sitzungsidentität. | Reset, Removal, Timeout, Cancel und Verbindungsverlust als unterschiedliche Zustände. |
| BUS | Pin-/Buskonfiguration, Bytes/Werte samt Einheit; Host-/Gerätezeitqualität getrennt. | Stop muss Helfer/Pin-/Busbesitz freigeben; gültiger Folgezustand pro Modus nachweisen. |
| BYTES | Originalbytes, Richtung, Sequenz, Portkonfiguration und Host-Zeit; Text ist nur eine Ansicht. | Ende/Timeout/USB-Umschaltung getrennt; Byteverluste dokumentieren. |
| CONTROL | Auftrags-ID, angefordert/angenommen/aktiv/beendet/Fehler; Ergebnisquelle. | Keine Erfolgsmeldung nur wegen gesendeter Bytes; Stopbestätigung oder unbekannter Gerätezustand. |
| FILE | Pfad, Größe, Format, Quellrevision/Hash und Vollständigkeit; Dateizeit ist kein Messzeitpunkt. | Teilübertragung markieren, Integrität prüfen, keinen ungesicherten Überschreib-Erfolg melden. |
| CAPTURE | Profil der Originaldaten, Sitzungsmanifest und Auswertungsrevision; unveränderte Rohdaten plus abgeleitete Sicht. | Automatisch speichern; abgebrochene Sitzung als solche wiederfinden. |
| SYNC | Objekt-ID, Basisrevision, Inhaltshash, Konflikt-/Verschlüsselungszustand. | Betroffenen Eintrag pausieren; andere Einträge weiter; Abbruch nicht als Upload-Erfolg. |
| LIMIT | Fehlender Mess-/Integrationsnachweis, kein künstlicher Datenstrom. | Als Grenze oder offenes Gate anzeigen, keine deaktivierte Schaltfläche als fertige Funktion zählen. |

Ressourcen in der CSV: `RPC` und `CLI` belegen den Steuerkanal exklusiv gegeneinander; `APP` den Loader-/App-Kontext; `IR`, `RF`, `NFC`, `LF`, `1WIRE`, `GPIO`, `UART`, `POWER` die jeweils betroffene Hardware; `SD` Dateizugriffe; `USBMODE`/`BLEMODE` Profilwechsel; `HELPER` genehmigte Geräte-Erweiterung; `HOST`/`NET` reine Desktop-/Netzarbeit. Kombinationen sind konservative Startannahmen, kein Beleg für gleichzeitigen Betrieb.

## Dashboard

| ID · Aufgabe | Stand / Weg | Host-Arbeit und Grenze | Beleg · Abnahme |
| --- | --- | --- | --- |
| D01 · Gerät, Firmware, Protokoll, SD und Speicher anzeigen | **C** · RPC DeviceInfo/Property/Storage | Vorhandene Modelle erweitern; Quelle und Stand je Wert sichtbar. | Q01 Q04 Q05 M-PROP M-SYS · A01 |
| D02 · Akku, Ladezustand und Powerdetails anzeigen | **C** · RPC pwrinfo/pwrdebug | Felder und Einheiten typisieren; unbekannte Werte fehlen statt 0. | Q01 M-PROP M-PROPDEF M-POWERHAL · A01 |
| D03 · Aktive Aufgaben, Verbindung und letzte Daten zeigen | **P** · Gemeinsames Desktop-Modell | Belegung und veraltete Werte zentral darstellen. | Q01 Q05 · A01 |
| D04 · Flipper-Bildschirm und Tasten bedienen | **C** · RPC GUI | Bestehenden Stream übernehmen; kein Messdatenersatz. | Q01 Q06 M-GUI · A01 |

## Dateien und Bibliothek

| ID · Aufgabe | Stand / Weg | Host-Arbeit und Grenze | Beleg · Abnahme |
| --- | --- | --- | --- |
| F01 · Dateien übertragen und verwalten | **C** · RPC Storage | Vorhandene Operationen nutzen; eindeutiger Fortschritt, Abbruch und Fehler. | Q01 M-STOR · A09 |
| F02 · Signale und Karten suchen, bearbeiten, vergleichen | **P** · Desktop-Dateiformate | Parser und Formatversionen je Dateityp; Originaldaten erhalten. | Q01 M-STOR · A09 |
| F03 · Aufnahmen importieren, exportieren und erneut auswerten | **P** · Lokales Sitzungsformat | Portables Format und Exportumfang noch spezifizieren. | Q02 · A02 |

## Infrarot

| ID · Aufgabe | Stand / Weg | Host-Arbeit und Grenze | Beleg · Abnahme |
| --- | --- | --- | --- |
| IR01 · Dekodierte Signale live empfangen | **C** · CLI ir rx | Protokoll, Adresse, Befehl, Wiederholungen erfassen. | M-IR · A04 |
| IR02 · RAW-Pulszeiten live anzeigen | **C** · CLI ir rx raw | Pulsdarstellung nach Frame-Empfang; kein optisches Frequenzspektrum. | M-IR W01 · A04 |
| IR03 · Gespeicherte Fernbedienung mit Tasten bedienen | **C** · App-RPC IR Load/Press/Release | App-Wrapper und Schema ergänzen; Tastenzustand bei Abbruch klären. | M-APP M-IRRPC Q01 Q07 · A04 |
| IR04 · Signale senden und Fernbedienungen bearbeiten | **C** · CLI TX / App-RPC + .ir-Dateien | Parameterprüfung und Host-Editor neu; Vorschau löst kein Senden aus. | M-IR M-IRRPC · A04 |
| IR05 · Aufnahmen vergleichen und dekodierte Muster auswerten | **P** · Desktop-Auswertung | Auf Basis der empfangenen Daten, kein erfundener Pegel. | M-IR W01 · A04 |
| IR06 · Optisches Amplituden-/Trägerfrequenzspektrum messen | **L** · Kein entsprechender interner Datenweg | Mit eingebautem demodulierendem Empfänger nicht als Messwert zusagen; Zusatzhardware wäre eigene Entscheidung. | W01 M-IR · A04 |

## Sub-GHz

| ID · Aufgabe | Stand / Weg | Host-Arbeit und Grenze | Beleg · Abnahme |
| --- | --- | --- | --- |
| RF01 · Dekodierte Pakete live empfangen | **C** · CLI subghz rx | Frequenz und gewähltes Modul erfassen, Parser je Protokoll. | M-RF W02 · A05 |
| RF02 · RAW-Pulse live empfangen | **C** · CLI subghz rx_raw | Vorzeichen/Dauer und Verlustmarker erhalten; geprüfter Pfad internes Funkmodul. | M-RF · A05 |
| RF03 · Gespeicherte Signale senden und stoppen | **C** · App-RPC / CLI tx_from_file | TX-Zustand, Stop und Fehlerrückgabe verbinden; nicht jede Datei beliebig wiederholbar. | M-RF M-RFRPC M-APP · A05 |
| RF04 · Dateien bearbeiten und RAW-Aufnahmen dekodieren | **C** · CLI decode_raw + Desktop-Editor | Dateiablauf und Protokollmetadaten; Rolling-State nicht blind überschreiben. | M-RF · A05 |
| RF05 · Frequenzabhängigen RSSI-Verlauf/Sweep anzeigen | **H** · JS subghz + js-CLI als Kandidat | Dwell-Zeit, Zeitstempel, unterstützte Bänder und Rate erst messen. | M-JS M-RFJS W02 · A03 A05 |
| RF06 · Externes CC1101 gezielt auswählen | **C** · CLI rx mit Geräteparameter | RAW-/TX-Pfade gesondert prüfen; nur tatsächlich erkanntes Modul anbieten. | M-RF · A05 |
| RF07 · Breitbandige I/Q-Samples wie bei einem SDR liefern | **L** · Kein solcher Quellpfad | Pulslängen und nacheinander gemessener RSSI sind keine simultanen I/Q-Daten. | M-RF M-RFJS W02 · A05 |

## NFC

| ID · Aufgabe | Stand / Weg | Host-Arbeit und Grenze | Beleg · Abnahme |
| --- | --- | --- | --- |
| NF01 · Karten und Protokolle erkennen | **C** · CLI nfc scanner | Erkennung ist noch kein vollständiger Karteninhalt. | M-NFCCLI M-NFSCAN · A06 |
| NF02 · Unterstützte Karten auslesen und speichern | **C** · CLI nfc dump → SD → RPC Storage | Protokollabhängig; gesperrte/ungelesene Teile als unvollständig markieren. | M-NFDUMP M-STOR · A06 |
| NF03 · APDU-Sequenz ausführen und Antworten auswerten | **C** · CLI nfc apdu | Vorab übergebene Liste; keine nachgewiesene dauerhafte interaktive PC/SC-Sitzung. | M-APDU · A06 |
| NF04 · Ausgewählte RAW-Protokolltransaktionen ausführen | **C** · CLI nfc raw | Nur unterstützte Protokolle; kein allgemeiner passiver NFC-Sniffer. | M-NFRAW · A06 |
| NF05 · Unterstützte Ultralight-Blöcke lesen/schreiben | **C** · CLI nfc mfu | Karten-/Blockgrenzen und Erfolg prüfen; keine universelle Schreibfunktion. | M-MFREAD M-MFWRITE · A06 |
| NF06 · Gespeicherte Karten emulieren | **C** · App-RPC LoadFile / CLI emulate | Dateiauswahl und Start getrennt; Protokollfähigkeit anzeigen. | M-NFEMU M-NFRPC M-APP · A06 |
| NF07 · Fortlaufende interaktive Kartensitzung halten | **H** · Eigene FAP + AppDataExchange als Kandidat | API/ABI, Kartenwechsel, Reset, Timeout und Stop im Prototyp beweisen. | M-APP M-APDU · A03 A07 |
| NF08 · Als Reader für andere Mac-Programme dienen | **O** · PC/SC-/OS-Bridge oder CCID-Helfer prüfen | Eigenes Integrationsgate; Debug-CCID mit Beispielantworten ist kein NFC-Reader. | W05 W06 W07 W08 M-CCIDFAM M-CCID M-CCIDCMD M-CCIDHAL · A07 |
| NF09 · Entsprechenden Systemreader unter Linux anbieten | **O** · pcsc-lite-/Reader-Integration prüfen | Linux-Zielumfang zusätzlich entscheiden; nicht aus Mac-Auswahl ableiten. | W08 M-APP · A07 |

## 125-kHz-RFID

| ID · Aufgabe | Stand / Weg | Host-Arbeit und Grenze | Beleg · Abnahme |
| --- | --- | --- | --- |
| LF01 · Schlüssel erkennen und lesen | **C** · CLI rfid read | Ergebnis nach Erkennung; Protokollabdeckung ausweisen. | M-LF · A08 |
| LF02 · Schlüssel emulieren und kompatible Tags beschreiben | **C** · CLI / App-RPC | Schreibziel und unterstützte Formate unterscheiden. | M-LF M-LFRPC M-APP · A08 |
| LF03 · RAW-Aufnahme auf SD erstellen und danach analysieren | **C** · CLI rfid raw_read/raw_analyze | Dateiaufnahme mit Overrun-Hinweis; nicht direktes Host-Live-Raw. | M-LF · A08 |
| LF04 · RAW-Daten fortlaufend am Desktop anzeigen | **H** · Eigener Helfer zu prüfen | Datenzugriff, Durchsatz und Verlustmarker noch nicht belegt. | M-LF M-APP · A03 A08 |

## iButton und 1-Wire

| ID · Aufgabe | Stand / Weg | Host-Arbeit und Grenze | Beleg · Abnahme |
| --- | --- | --- | --- |
| IB01 · iButton-Schlüssel lesen | **C** · CLI ikey read | Protokoll und Daten lesbar darstellen. | M-IB · A08 |
| IB02 · iButton emulieren / passende Schlüssel schreiben | **C** · CLI ikey / App-RPC | Write nur für unterstützte beschreibbare Typen; Stop bestätigen. | M-IB M-IBRPC M-APP · A08 |
| IB03 · 1-Wire-Geräte suchen | **C** · CLI onewire | Bus-Suche ist kein universeller Protokollanalysator. | M-1W · A08 |

## GPIO und Busse

| ID · Aufgabe | Stand / Weg | Host-Arbeit und Grenze | Beleg · Abnahme |
| --- | --- | --- | --- |
| GP01 · Digitale Pins konfigurieren, lesen und setzen | **C** · RPC GPIO / CLI gpio | RPC-Wrapper ergänzen; Pinmodi/-namen als Auswahl. | M-GPIOCLI M-GPIORPC Q01 · A10 |
| GP02 · OTG-Versorgung steuern | **C** · RPC GPIO OTG | Zustand und Wirkung auf belegte Module sichtbar machen. | M-GPIORPC · A10 |
| GP03 · Analoge Werte, Interrupts und PWM nutzen | **H** · JS gpio + js-CLI | Scripthülle, Einheit/Kalibrierung, Stop und Raten prüfen. | M-JS M-GPIOJS · A03 A10 |
| GP04 · I²C-Geräte finden und Transaktionen ausführen | **H** · JS i2c | Host-Editor/Transferformat; aktiv angesprochener Bus, kein passiver Sniffer-Nachweis. | M-JS M-I2C · A03 A10 |
| GP05 · SPI-Transaktionen ausführen | **H** · JS spi | Busbelegung und Freigabe; Transaktion statt ungeprüftem passivem Sniffing. | M-JS M-SPI · A03 A10 |
| GP06 · UART-Terminal und Protokollansicht nutzen | **H** · JS serial als Kandidat | Bytes verlustarm erfassen, Einstellungen und Abbruch testen. | M-JS M-SERIAL · A03 A10 |
| GP07 · Flipper als USB-UART-Brücke nutzen | **C** · Geräte-App USB-UART | RPC wird beendet; USB-Profil/Portwechsel, Rückkehr und Besitz neu koordinieren. | M-UARTAPP M-UARTCLOSE M-UART · A10 |
| GP08 · Digitale Ereignisse als Zeitverlauf analysieren | **H** · GPIO-Interrupt-Helfer zu prüfen | Keine zugesagte Logic-Analyzer-Samplerate; Zeitbasis/Verluste messen. | M-GPIOJS M-APP · A03 A10 |

## USB, Bluetooth und BadKB

| ID · Aufgabe | Stand / Weg | Host-Arbeit und Grenze | Beleg · Abnahme |
| --- | --- | --- | --- |
| US01 · BadUSB-/BadKB-Skripte bearbeiten und übertragen | **P** · Desktop-Editor + RPC Storage | Syntax, Dateien und USB-/BLE-Zielmodus unterscheiden. | M-BADAPP M-BAD M-BADWORK M-STOR · A11 |
| US02 · BadKB starten, pausieren, stoppen und Status erhalten | **O** · App-Steuerung/Helfer prüfen | UI-Tastendruck vorhanden, stabile strukturierte Steuerung nicht belegt; VCP kann entfallen. | M-BAD M-BADWORK Q06 · A11 |
| US03 · USB-Speichermodus aus Image verwenden | **H** · JS usbdisk | Image-Belegung, Profilwechsel und Rückkehr in die Steuerverbindung prüfen. | M-JS M-DISK · A03 A11 |
| US04 · Unterstützte BLE-Beacon-Funktionen steuern | **H** · JS blebeacon | Konfigurierbare Advertisements; kein allgemeiner BLE-Scanner oder Sniffer. | M-JS M-BEACON · A03 A11 |
| US05 · Verfügbare Bluetooth-Diagnosen einordnen | **C** · CLI bt | Testmodi mit eigener BLE-Belegung; keine fremden Paketmitschnitte versprechen. | M-BT · A11 |
| US06 · Universellen USB-/BLE-Busmitschnitt bereitstellen | **O** · Kein allgemeiner Pfad belegt | Konkreten Anwendungsfall und Hardware/API zuerst klären; keine SDR/HCI-Fähigkeit erfinden. | M-BT M-BAD M-DISK M-BEACON · A11 |

## Apps und Erweiterungen

| ID · Aufgabe | Stand / Weg | Host-Arbeit und Grenze | Beleg · Abnahme |
| --- | --- | --- | --- |
| AP01 · Installierte Apps finden, starten und beenden | **C** · CLI Loader / RPC App | Host-Wrapper ergänzen; App-RPC/Close nicht für jede App verfügbar; ggf. manuell beenden. | M-LOAD M-APP Q01 Q07 · A12 |
| AP02 · JavaScript-Dateien starten und Ausgabe aufnehmen | **C** · CLI js <path> | Stop per Ctrl+C, Skriptfehler und Ausgabegrenzen sichtbar. | M-JS M-JSTHREAD · A03 A12 |
| AP03 · Eigene Hilfsprogramme nach Freigabe installieren | **H** · Storage + FAP/JS + App-RPC/CLI | Jede Installation und jede neue Helferversion ausdrücklich freigeben; Version, Zielpfad und Zweck anzeigen. | M-APP M-STOR M-JS · A03 |
| AP04 · Weitere Geräte-Apps und externe Module anbinden | **O** · Pro App-/Hardwareprofil inventarisieren | Keine pauschale Komplettunterstützung aller Drittanbieter-Apps; kein stilles Streichen des Zielbereichs. | M-APP M-LOAD · A12 |

## Pflege, Firmware und Backups

| ID · Aufgabe | Stand / Weg | Host-Arbeit und Grenze | Beleg · Abnahme |
| --- | --- | --- | --- |
| CA01 · Neustart, Zeit, Recovery und vorhandene Pflege nutzen | **C** · RPC System / DFU | Bestehende Pfade erhalten, Zustandswechsel und Fehler in den Ablauf integrieren. | Q01 Q13 M-SYS · A13 |
| CA02 · Firmware-Angebote im Programm anzeigen und laden | **P** · Quellenadapter + bestehender Download | Official/Momentum zuerst belegen; fremde Quellen mit eigenem Format, Target und Identität. | Q14 Q15 Q16 W03 W04 · A14 |
| CA03 · Firmware installieren/wechseln/aktualisieren | **C** · Vorhandener TGZ-/DFU-Pfad | Neue Backup-Vorkette und Abschlussprüfung; Firmware- und App-Updater getrennt. | Q13 Q14 Q17 · A14 |
| CA04 · Manuelle Gerätesicherung erstellen | **P** · Storage + eigener Backup-Manifestablauf | /int und /ext inventarisieren; vorhandener qFlipper-Backup nur /int. | Q08 Q09 Q12 M-STOR · A15 |
| CA05 · Automatisch vor Update, Wechsel und Restore sichern | **P** · Gemeinsame Backup-Vorkette | Bei Backupfehler Update/Wechsel/Restore anhalten, bis ein geprüftes Backup vorliegt; kein Bypass. | Q08 Q13 Q14 U01 · A15 |
| CA06 · Backup prüfen und selektiv wiederherstellen | **P** · Storage + eigener Restore-Plan | Kompatibilität, Dateiprüfung und Teilfehler; kein atomarer Geräte-Rollback zugesagt. | Q10 Q11 M-STOR · A15 |
| CA07 · Backups/Aufnahmen nach Grenzen verwalten | **P** · Host-Katalog | Alter/Anzahl/Größe wählbar; automatische Bereinigung zunächst aus. | U01 · A16 |

## Cloud, MCP und Oberfläche

| ID · Aufgabe | Stand / Weg | Host-Arbeit und Grenze | Beleg · Abnahme |
| --- | --- | --- | --- |
| CL01 · Alles einschließlich übertragbarer Einstellungen synchronisieren | **P** · Host-Sync mit Anbieteradaptern | Google Drive, Dropbox, OneDrive, WebDAV und S3 im ersten Release; Konflikt je Eintrag pausieren; Löschungen überall in den Papierkorb, endgültig manuell. Zugangsdaten separat festlegen; MCP-Freigabe bleibt lokal. | U01 W09 W10 W11 W12 W13 · A16 |
| CL02 · Pro Cloud-Verbindung Verschlüsselung wählen | **P** · Host-Sync | Schlüsseldatei/-code oder Masterpasswort wählbar; Kryptografie, Schlüsselverwaltung und Umstellung noch festlegen. | U01 · A16 |
| CL03 · MCP mit Gesamtfreigabe je Computer nutzen | **P** · Gemeinsamer lokaler Backend-Zugang | Gleiche Belegung/Stop/Fehler wie UI; Endpoint und Authentisierung noch entwerfen. | U01 Q01 · A17 |
| CL04 · Drei Layouts und Easy Mode anbieten | **P** · Qt/QML-Arbeitsfläche des Forks | Erster Start mit fester Seitenleiste und Arbeits-/Detailbereich; alle drei Layouts wählbar, gleiche Aktionen im Easy Mode. | U01 · A18 |
