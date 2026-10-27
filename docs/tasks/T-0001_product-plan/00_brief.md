# T-0001 · uFlipper: abgestimmtes Zielbild

Stand: 26.10.2026, ergänzt am 27.10.2026 · Produktplanung, keine Implementierungsfreigabe.

uFlipper soll den Flipper Zero zu einem professionellen, vom Computer bedienbaren Werkzeug machen. Der Computer übernimmt Arbeitsfläche, Darstellung, Auswertung und Organisation; der Flipper stellt die Hardware bereit. Grundlage ist der qFlipper-Fork. Die Zielplattformen sind macOS und Linux.

IR und NFC waren Beispiele für ein übergreifendes Prinzip: alle Gerätebereiche gehören zum Zielumfang. Gewünschte Abläufe umfassen Erfassen, Live-Empfang, Konfigurieren, Ausführen, Analysieren, Bearbeiten und Organisieren, jeweils passend zur Funktion und den verfügbaren Hardwaredaten.

Die erste Version soll mit unveränderter Momentum-Firmware arbeiten. Eigene Hilfsprogramme und Skripte auf der SD-Karte sind nach ausdrücklicher Freigabe erlaubt (Entscheidung 16 vom 27.10.2026); eine jetzige Installation wurde nicht freigegeben. Ob damit jede gewünschte Funktion erreichbar ist, ist noch nicht nachgewiesen. Keine stillschweigende Reduktion des Umfangs und keine Zusage technisch ungeprüfter Funktionen.

Verbindliche Produktentscheidungen: [01_scope.md](01_scope.md). Abnahmekriterien und nächste Prüfungen: [02_done_when.md](02_done_when.md). Arbeitsgrenzen: [03_constraints.md](03_constraints.md).

Der Nutzer verlangt transparente, gemeinsam abgestimmte Schritte. Die Auswahl der Produkteigenschaften erteilt keine Freigabe für Codeänderungen, Geräteaktionen, Installationen, Cloud-Verbindungen oder Veröffentlichungen.

Der technische Entwurf mit Funktionsmatrix und Etappen liegt unter [T-0002](../T-0002_technical-plan/00_brief.md). Er ist noch gemeinsam zu prüfen und kein Laufzeitnachweis.

## Weitere Auswahlrunde · 27.10.2026

Entscheidungen 17–22 sind ausdrücklich bestätigt: feste Seitenleiste als Startansicht; Update/Wechsel/Restore bis zu einem geprüften Backup anhalten; Google Drive, Dropbox, OneDrive, WebDAV und S3 bereits im ersten Release; Löschungen überall in den Papierkorb und endgültig nur manuell; Wiederherstellungsschlüssel und Masterpasswort als wählbare Zugangswege für verschlüsselte Daten; jede neue Helferversion erneut ausdrücklich freigeben. Alle drei Layouts bleiben auswählbar, automatische Bereinigung bleibt zunächst aus.

Dies konkretisiert Produktverhalten. Es startet keine Implementierung, Cloud-Verbindung, Installation oder Geräteaktion.
