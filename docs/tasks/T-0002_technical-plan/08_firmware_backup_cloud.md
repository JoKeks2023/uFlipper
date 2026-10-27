# Firmware-Bibliothek, Backupper und Cloud

Die technische Umsetzung der Abläufe ist ein Vorschlag zur Prüfung; ausdrücklich bestätigte Produktregeln sind verbindlich. Verbindliche Nutzerentscheidungen bleiben in [T-0001](../T-0001_product-plan/01_scope.md); offene Auswahlpunkte in [01_scope.md](01_scope.md). Quellenkürzel beziehen sich auf [10_sources.md](10_sources.md).

## Firmware direkt in der App

Bestätigt: offizielle Firmware, Momentum, frei erweiterbare Quellen; Finden, Download und Aktualisierung ohne notwendiges Wechseln auf eine Website.

Die Metadatenwege sind vorhanden: offizielles Verzeichnis W04 mit Release-/RC-/Development-Kanälen, Momentum-Releases W03 mit Updatepaketen. Momentaufnahme 27.10.2026, kein Paket installiert. Beispielasset: `flipper-z-f7-update-mntm-012.tgz`; der beim Abruf angegebene SHA256 ist `1d9e1e600c8680172ff6f6913ca0b12484e7d45adbc72a66552e534784c8c1a7`. Metadaten und Paket vom gleichen Anbieter sind kein unabhängiger Signaturnachweis.

Vorschlag für Quellenadapter: Quelle/Projekt-ID, Anzeigename, Herkunft, Kanal, Versionslabel, Revision, Zielhardware, Paketart, URL, Größe/Hash und angebotene Signatur getrennt erfassen. „Frei erweiterbar“ über deklarative Quellenkonfiguration mit Auswahl bekannter Formate und „Andere…“ planen; keine beliebige entfernte Programmausführung voraussetzen. Exaktes Erweiterungsformat erst spezifizieren. Versionen innerhalb eines Projekts vergleichen; `mntm-012` und eine offizielle Versionsnummer ergeben keine gemeinsame numerische Rangfolge (Q16).

Vorgeschlagener Ablauf:

1. Quelle/Kanal/Version auswählen; installierte Firmware und Zielpaket gegenüberstellen. Nicht unterstützte Hardware/Paketformate vor Geräteänderung erklären.
2. Paket herunterladen, erwartete Integrität prüfen, Provenienz/Prüfstatus anzeigen und Cache führen. Fehlende oder nicht überprüfbare Prüfsummen klar unterscheiden; Fehlerpolitik vor Release konkretisieren.
3. Betroffene Daten/Apps und etwaigen Firmwarewechsel anzeigen. Aktive Messung erst nach Konfliktentscheidung beenden.
4. Automatisches Backup gemäß gewähltem Profil erstellen und prüfen. Bestätigte Regel: Bei Fehler Update/Firmwarewechsel/Restore anhalten, bis ein geprüftes Backup vorliegt. Kein Fortfahren ohne Backup; UI und MCP verwenden dieselbe Sperre.
5. Vorhandenen TGZ-/SD-Updater oder passenden DFU-/Recovery-Pfad anbinden. Region-/Targetprüfungen erhalten; keine Beschränkungen umgehen.
6. Nach Neustart Gerät erneut erkennen, tatsächlich gemeldete Version, SD und Fähigkeiten prüfen. Empfangene Startbestätigung allein ist kein Update-Erfolg. Bei Recovery verbleibenden Zustand und nächste mögliche Handlung zeigen.

Ein Firmwarepaket im Cache ist keine Geräte-Sicherung. Ein heruntergeladenes altes Paket garantiert keinen automatischen Rollback. Kompatibilität von Settings/Apps/Helfern nach Firmwarewechsel prüfen.

uFlippers eigener App-Updater muss separat werden: Q17 verweist derzeit auf den qFlipper-Updatekanal. Ein unveränderter Updater darf später nicht versehentlich wieder das Upstream-Programm als uFlipper-Update ausliefern. Neuer Kanal und Verhalten sind O11; heute keine Konfigurationsänderung.

## Backupper: Inhalt zuerst sichtbar machen

Q08/Q09 sichern den internen Storage `/int`. SD-Dateien sind dadurch nicht vollständig geschützt; Q13 enthält außerdem einen speziellen temporären Sicherungspfad beim Firmwareinstallieren. Beides ersetzt keinen dauerhaft verwalteten vollständigen Benutzerbackup.

Vorgeschlagenes Inventar vor Festlegung von O03:

| Bereich | Zu erfassen/prüfen | Grenze |
| --- | --- | --- |
| Interner Storage | Lesbare Dateien einschließlich versteckter Einstellungen; Dateiformat-/Firmwarebezug. | Kein Beleg für vollständiges Flash-/OTP-/Secure-Element-Abbild. |
| SD-Nutzerdaten | Signale, Karten, Skripte, Aufnahmen, relevante Anwendungsdaten. | Lesbarkeit und mögliche Änderungen während des Transfers prüfen. |
| SD-Apps und Assets | Apps, Bibliotheken, Ressourcen, nötige Versionen/Hashes. | Kompatibilität mit anderer Firmware/SDK nicht garantiert. |
| Updatepakete/Caches | Separat inventarisieren und im Profil ausdrücklich ein-/ausschließen. | Ausschluss spart Platz, ist aber keine bereits getroffene Nutzerentscheidung. |
| Desktop-Daten | Lokale Aufnahmen, Bibliothek, Zuordnungen und Einstellungen. | Eigener Scope im Backup-Manifest, kein Flipper-Dateisystem vortäuschen. |

Backup-Manifest vorschlagen: Sicherungs-ID, Gerät, Firmware/Revision/RPC, Profil, Start-/Endzeit, Dateiliste mit Scope/Pfad/Größe/Hash, Vollständigkeit, Fehler und Konsistenzzustand. Formatversion und kompatible Restore-Ziele festlegen. Originalarchiv behalten; Prüfresultate getrennt protokollieren.

Konsistenzverfahren noch nachweisen: schreibende Geräte-App soweit nötig ruhigstellen, Inventar erfassen, Dateien lesen, Abschlussinventar/Hashes vergleichen. Ein Datensatz, der währenddessen verändert wurde, muss erneut gelesen oder als instabil gekennzeichnet werden. Kein atomarer Snapshot des ganzen Gerätes über mehrere RPC-Dateizugriffe versprochen. Geräteseitige Änderungen durch physische Bedienung berücksichtigen.

Erfolg erst nach komplettem Lesen, Archivabschluss, lokalem Wiederöffnen und Hashprüfung. Abziehen, voller Datenträger, fehlende SD und nicht lesbare Dateien erzeugen unvollständige/fehlgeschlagene Sicherung. Cloud-Upload ist nachgelagert; ob ein Update auf Cloud-Abschluss warten muss, ist nicht entschieden. Vorschlag: lokales geprüftes Backup genügt, Cloud-Status getrennt zeigen.

## Wiederherstellung

Backup auswählen → Inhalte/Quelle/Kompatibilität prüfen → Änderungen und mögliche Überschreibungen anzeigen → automatisches Backup des aktuellen Standes erstellen → selektiver Restore → Zielinhalt erneut lesen/vergleichen → ggf. Neustart und Kontrolle.

„Vor Restore sichern“ darf nicht rekursiv einen weiteren Restore starten. Fehler beim Sicherheitsbackup halten den Ablauf an, bis ein geprüftes Backup vorliegt (bestätigte Entscheidung 18). Wiederherstellung ist bei den vorhandenen Dateizugriffen nicht atomar: Fortschritt pro Datei, bestehende/ersetzte/fehlgeschlagene Einträge und verbleibenden Teilzustand protokollieren. Keine pauschale Erfolgsmeldung nach bloßem Upload.

Rückspielen über Firmwarefamilien/-versionen: Nutzdateien und Firmware-Einstellungen getrennt bewerten. Unbekannte Settings nicht blind übernehmen; Migration/teilweiser Restore erst mit Kompatibilitätsmatrix anbieten. Wiederherstellung von Firmware selbst ist ein eigener Ablauf und darf nicht aus einem Dateibackup abgeleitet werden.

## Cloud: fünf Anbindungen für den ersten Release bestätigt

Bestätigt sind **Google Drive, Dropbox und OneDrive sowie WebDAV und S3 bereits im ersten öffentlichen Release**, automatische Synchronisierung aller Inhalte einschließlich Einstellungen, Konfliktentscheidung je betroffenem Eintrag und ausdrückliche Verschlüsselungswahl je neuer Verbindung. Es ist kein eigener uFlipper-Cloud-Dienst beauftragt.

| Gewählte Release-Anbindung | Ansatz aus der Quellenprüfung | Vor Implementierung/Abnahme nachweisen |
| --- | --- | --- |
| Google Drive | App-Datenbereich über Anbieter-API (W09). | App-Registrierung/Anmeldung, Speicherzugriff, Löschverhalten und Datei-/Ordnertransparenz. |
| Dropbox | Scopes und App-Zugriff (W10). | App-Zulassung, Anmeldung, Quoten, bedingte Änderungen und Konflikte. |
| OneDrive | Graph-AppFolder (W11). | Kontotypen, Einwilligung/Registrierung, Revisionen und große Objekte. |
| WebDAV (Nextcloud als Dokumentationsbeispiel) | DAV-Dateioperationen/ETags (W12). | Konkrete Server, Authentisierung, Sperren/bedingte Writes und unterbrochene Übertragung. |
| S3 / kompatibler Speicher | Objektzugriff und bedingte Writes (W13). | Endpoint-/Versionierungsverhalten je Anbieter; ETag nicht als universellen SHA256 behandeln. |

Alle fünf Anbindungen sind als Release-Umfang ausdrücklich gewählt; technische Adapter, Kontotypen und unterstützte eigene Server müssen noch implementiert und geprüft werden. Die Quellen belegen Integrationsansätze, keine fertigen uFlipper-Anbindungen. Anmeldung kann eine Anbieter-Webfreigabe benötigen; sie ist nicht mit dem Wunsch zu verwechseln, Firmware ohne Websitebesuch verwalten zu können.

Vorgeschlagenes Synchronisationsmodell:

- Lokale Inhalte zuerst dauerhaft speichern. Große Aufnahmen/Backups blockweise übertragen und nach Unterbrechung fortsetzen; Inhaltshash und veröffentlichte Manifestrevision trennen.
- Objekt-ID, Basisrevision, Inhalt, Metadaten und Löschmarkierung getrennt führen. Erkennt ein Computer eine Änderung gegenüber derselben Basis, den betroffenen Eintrag pausieren und beide Fassungen erhalten. Andere Einträge weiter synchronisieren. Keine automatische „neueste gewinnt“-Regel.
- Einstellungen als eigene Einträge/Gruppen modellieren, damit ein Layout-Konflikt nicht sämtliche Geräteaufnahmen blockiert. Gruppierungsgranularität vor Implementierung festlegen.
- Ziel „alles einschließlich Einstellungen“ sichtbar einlösen: Inventar aller Einstellungsarten führen. Für lokale Pfade, Computerbindung, Zugangsdaten und Schlüssel O04 entscheiden, statt diese kommentarlos auszulassen oder auf einem fremden Computer aktiv zu verwenden. MCP-Freigabe bleibt zwingend pro Computer; ein synchronisierter Einstellungsstand aktiviert sie nie.
- Synchronisation und Datensicherung getrennt benennen. Bestätigt: Löschungen verschieben den Eintrag auf allen Computern und in der Cloud in einen Papierkorb; endgültige Löschung ist eine gesonderte manuelle Aktion. Keine automatische Löschfrist vereinbart. Gleichzeitige Bearbeitung/Löschung pausiert den betroffenen Eintrag zur Auswahl. Technische Regeln für Offline-Computer und Wiederherstellung aus dem Papierkorb bleiben O06.

## Optionale Verschlüsselung

Bei Einrichtung jeder Verbindung eine erforderliche Auswahl „vor Upload verschlüsseln“ / „unverschlüsselt“ ohne vorausgewählte automatische Zustimmung. Mit verständlicher Erklärung von Zugriff, Recovery und Wechsel. Implementierung erst nach O05:

- Beide gewählten Zugangswege anbieten: Wiederherstellungsschlüssel als Datei/Code oder selbst gewähltes Masterpasswort. Das Verfahren als Auswahl anbieten; keine Pflicht, beide gemeinsam zu benutzen, und noch kein Standardverfahren festgelegt. Sichere lokale Ablage, Schlüsselverlust, Wiederherstellung auf einem zweiten Computer, Wechsel zwischen Verfahren und Entzug eines Computers technisch definieren.
- Festlegen, ob Namen/Größen/Metadaten sichtbar bleiben. Nicht „Ende-zu-Ende verschlüsselt“ versprechen, solange Format, Schlüsselpfad und Metadatenmodell nicht geprüft sind.
- Wechsel der Einstellung als Migration behandeln: vorhandene Objekte, temporäre Kopien, alte unverschlüsselte Versionen und unterbrochene Migration explizit berücksichtigen. Ein Schalter kann keine frühere Klartextkopie automatisch zurückholen.
- Konfliktentscheidung muss mit passenden Schlüsseln möglich sein; falscher/fehlender Schlüssel ist kein leerer Datensatz.

## Aufbewahrung

Alter, Anzahl und Speicherverbrauch als auswählbare Grenzen; automatische Bereinigung zunächst aus. Belegter Inhalt, vorgemerkte Löschung und Cloud-Stand sichtbar zeigen. Die Papierkorb-Regel ist bestätigt: Löschungen überall in den Papierkorb, endgültig erst manuell. Mindestkopien, angeheftete Sicherungen, Offline-Abgleich und technische Wiederherstellung aus dem Papierkorb sind noch festzulegen. Keine automatische Papierkorb-Leerung oder endgültige Cloud-Löschung aus einer Alters-/Größengrenze ableiten.
