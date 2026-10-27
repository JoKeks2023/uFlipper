# Prüfung

Noch kein Laufzeitnachweis der neuen Oberfläche. Vorheriger Paketstand `290f6ce` startet auf Intel und Apple Silicon; dies wird nicht als Nachweis des neuen UI-Stands verwendet.

- CI `b9b0f96` / Run `40569613446`: Core auf beiden Plattformen bestanden. Linux-App-Build scheitert an einer durch das Voranstellen eines Includes in Zeile 2 verschobenen UTF-8-BOM in `applicationbackend.cpp`. BOM entfernt; frühe Compilerprüfung auf alle geänderten Integrationsdateien erweitert. Noch keine grüne neue Oberfläche behauptet.

- CI `888e2b3` / Run `40570303800`: Linux-App und Qt-Verträge inklusive Renderthread-Loggerprüfung bestanden; tatsächlicher Start und Dashboard-Screenshot erzeugt. Mausklickprüfung erreicht die IR-Seite nicht. Eingabetreiber auf QtTest-Mausereignisse umgestellt und Fehlerbeleg um betroffenen Button/Seite sowie Fehler-Screenshot erweitert; Erfolg noch nicht behauptet.
- Ersten nativen Dashboard-Screenshot selbst angesehen: keine Daten erfunden, aber zusätzliche ScrollView-Rahmen gegenüber dem Entwurf. Rahmen nur in der neuen Workbench deaktiviert; bestehende Datei-/Logflächen behalten ihre Gestaltung.

- CI `4b0025b` / Run `40571723174`: kompletter Linux-App-Build, Qt-Vertragstests und nativer UI-Lauf erfolgreich. **12 echte QtTest-Mausklicks**, Easy-Mode-Einstellung geändert und wiederhergestellt, alle fünf Settings-Tabs erreicht, zehn native Screenshots gespeichert, 1060×680 geprüft, keine erfassten QML-Laufzeitfehler. Der vorherige Fehllauf lag am manuellen Eingabetreiber, nicht an der Navigationsbindung.
- Dashboard, IR, allgemeine Settings und kompakte Ansicht selbst visuell angesehen. Letzte Nacharbeit: Tabs mit Unterstrich wie freigegeben, zurückhaltende Dropdowns, vollständig lesbares Maximieren-Symbol, Tooltips bei langen Navigationstiteln. Neuer Paket-/Sichttest nach diesen Änderungen erforderlich.
