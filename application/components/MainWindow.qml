import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15
import Theme 1.0
import QFlipper 1.0
import "workbench"
Item {
    id: shell
    objectName: "workbench"
    anchors.fill: parent
    property string currentPage: "dashboard"
    property bool showLogs: false
    property string pendingPage: ""
    property bool quitAfterNfcStop: false
    property bool quitAfterBackupStop: false
    function navigate(page) {
        if (Backend.backup.blocked && ["screen", "library", "firmware", "nfc"].indexOf(page) >= 0) {
            backupConflict.open()
            return
        }
        if (Backend.nfc.blocked && ["screen", "library", "firmware", "backups"].indexOf(page) >= 0) {
            if (!Backend.nfc.active) { nfcBlocked.open(); return }
            pendingPage = page
            nfcConflict.open()
            return
        }
        currentPage = page
    }
    readonly property bool maintenance: Backend.backendState > ApplicationBackend.ScreenStreaming && Backend.backendState < ApplicationBackend.Finished
    readonly property bool busy: maintenance || Backend.fileManager.isBusy
    readonly property bool resultVisible: Backend.backendState >= ApplicationBackend.Finished
    readonly property var device: Backend.deviceState ? Backend.deviceState.info : null
    readonly property var navigation: [
        {key:"dashboard",title:qsTr("Übersicht")}, {key:"screen",title:qsTr("Gerätebildschirm")},
        {key:"library",title:qsTr("Dateien")}, {key:"firmware",title:qsTr("Firmware")}, {key:"backups",title:qsTr("Backups")},
        {key:"infrared",title:qsTr("Infrarot")}, {key:"subghz",title:qsTr("Sub-GHz")}, {key:"nfc",title:qsTr("NFC")},
        {key:"rfid",title:qsTr("125-kHz-RFID")}, {key:"ibutton",title:qsTr("iButton / 1-Wire")},
        {key:"gpio",title:qsTr("GPIO und Busse")}, {key:"usb",title:qsTr("USB und Bluetooth")}, {key:"apps",title:qsTr("Apps und Skripte")}
    ]
    Rectangle { id: shellBg; anchors.fill: parent; color: "#080400"; border.color: Theme.color.mediumorange2 }
    Rectangle {
        id: titlebar
        height: 40; anchors.top: parent.top; anchors.left: parent.left; anchors.right: parent.right
        color: "#000000"
        Rectangle { anchors.bottom: parent.bottom; width: parent.width; height: 1; color: Theme.color.mediumorange2 }
        DragHandler { target: null; onActiveChanged: if (active) shell.Window.window.startSystemMove() }
        RowLayout {
            anchors.fill: parent; anchors.leftMargin: 20; anchors.rightMargin: 16; spacing: 16
            WorkbenchLabel { text: "uFlipper"; font.pixelSize: 26 }
            WorkbenchLabel { text: qsTr("ENTWICKLUNGSVERSION"); font.pixelSize: 12; opacity: 0.5 }
            Item { Layout.fillWidth: true }
            WorkbenchButton { text: "_"; implicitWidth: 40; navigation: true; leftPadding: 0; rightPadding: 0; Accessible.name: qsTr("Minimieren"); onClicked: shell.Window.window.showMinimized() }
            WorkbenchButton { text: "[ ]"; implicitWidth: 40; navigation: true; leftPadding: 0; rightPadding: 0; Accessible.name: qsTr("Fenstergröße ändern"); onClicked: shell.Window.window.visibility === Window.Maximized ? shell.Window.window.showNormal() : shell.Window.window.showMaximized() }
            WorkbenchButton { text: "X"; implicitWidth: 40; navigation: true; leftPadding: 0; rightPadding: 0; enabled: !shell.busy; Accessible.name: qsTr("Beenden"); onClicked: shell.Window.window.close() }
        }
    }
    Rectangle {
        id: sidebar
        anchors.top: titlebar.bottom; anchors.left: parent.left; anchors.bottom: footer.top
        width: 184; color: "#000000"
        Rectangle { anchors.right: parent.right; width: 1; height: parent.height; color: Theme.color.mediumorange2 }
        ColumnLayout {
            anchors.fill: parent; anchors.margins: 8; spacing: 8
            WorkbenchLabel { text: qsTr("ARBEITSPLATZ"); font.pixelSize: 12; opacity: 0.65; Layout.margins: 8 }
            ScrollView {
                padding: 0
                outlined: false
                Layout.fillWidth: true; Layout.fillHeight: true; clip: true
                contentWidth: availableWidth
                ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
                ColumnLayout {
                    width: parent.width; spacing: 0
                    Repeater {
                        model: shell.navigation
                        ColumnLayout {
                            required property var modelData
                            required property int index
                            Layout.fillWidth: true; spacing: 8
                            WorkbenchLabel { visible: index === 5; text: qsTr("WERKZEUGE"); font.pixelSize: 12; opacity: 0.65; Layout.leftMargin: 8; Layout.topMargin: 16 }
                            WorkbenchButton {
                                objectName: "nav-" + modelData.key
                                text: modelData.title; navigation: true; checked: shell.currentPage === modelData.key
                                enabled: !shell.busy; Layout.fillWidth: true
                                onClicked: shell.navigate(modelData.key)
                            }
                        }
                    }
                }
            }
            WorkbenchRule {}
            WorkbenchButton { objectName: "nav-settings"; text: qsTr("Einstellungen"); navigation: true; checked: shell.currentPage === "settings"; Layout.fillWidth: true; enabled: !shell.busy; onClicked: shell.navigate("settings") }
        }
    }
    Rectangle {
        id: connectionbar
        anchors.top: titlebar.bottom; anchors.left: sidebar.right; anchors.right: parent.right
        height: 56; color: "#100800"
        Rectangle { anchors.bottom: parent.bottom; height: 1; width: parent.width; color: Theme.color.mediumorange2 }
        RowLayout {
            anchors.fill: parent; anchors.leftMargin: 24; anchors.rightMargin: 24; spacing: 16
            WorkbenchLabel { text: shell.device ? "Flipper Zero · " + shell.device.name : qsTr("Flipper Zero"); font.pixelSize: 16 }
            Rectangle { width: 7; height: 7; radius: 4; color: Backend.deviceState && Backend.deviceState.isOnline ? Theme.color.lightgreen : Theme.color.mediumorange1 }
            WorkbenchLabel { text: Backend.deviceState && Backend.deviceState.isOnline ? Backend.deviceState.isRecoveryMode ? qsTr("Wiederherstellungsmodus") : qsTr("USB verbunden") : Backend.isQueryInProgress ? qsTr("Gerät wird gelesen …") : qsTr("Nicht verbunden"); color: Backend.deviceState && Backend.deviceState.isOnline ? Theme.color.lightgreen : Theme.color.lightorange2; font.pixelSize: 15 }
            Item { Layout.fillWidth: true }
            WorkbenchButton { text: qsTr("Vollbackup läuft"); visible: Backend.backup.active; onClicked: shell.navigate("backups") }
            WorkbenchButton { text: Backend.nfc.active ? qsTr("NFC-Sitzung läuft") : qsTr("NFC-Stop ungeklärt"); visible: Backend.nfc.blocked; onClicked: shell.navigate("nfc") }
            WorkbenchButton { text: qsTr("Aktualisieren"); enabled: !shell.busy; onClicked: { Backend.telemetry.refresh(); Backend.workbench.refresh() } }
        }
    }
    Item {
        id: workspace
        anchors.top: connectionbar.bottom; anchors.left: sidebar.right; anchors.right: parent.right; anchors.bottom: logPanel.visible ? logPanel.top : footer.top
        Loader {
            id: pageLoader
            anchors.fill: parent; visible: !shell.maintenance && !shell.resultVisible
            sourceComponent: {
                if (shell.currentPage === "dashboard") return dashboardComponent
                if (shell.currentPage === "nfc") return nfcComponent
                if (shell.currentPage === "settings") return settingsComponent
                if (shell.currentPage === "screen") return screenComponent
                if (shell.currentPage === "library") return filesComponent
                if (shell.currentPage === "backups") return backupsComponent
                if (shell.currentPage === "firmware") return firmwareComponent
                return toolComponent
            }
        }
        Connections { target: pageLoader.item; ignoreUnknownSignals: true; function onNavigate(page) { shell.navigate(page) } }
        Item {
            anchors.centerIn: parent; width: Math.min(800, parent.width - 16); height: 394
            visible: shell.maintenance || shell.resultVisible
            UpdateOverlay { anchors.fill: parent; backgroundRect: shellBg; opacity: shell.maintenance ? 1 : 0 }
            FinishOverlay { anchors.fill: parent; backgroundRect: shellBg; opacity: shell.resultVisible ? 1 : 0 }
        }
    }
    Rectangle {
        id: logPanel
        visible: shell.showLogs
        height: 160; anchors.left: sidebar.right; anchors.right: parent.right; anchors.bottom: footer.top
        color: "#000000"; border.color: Theme.color.mediumorange2
        TextView { anchors.fill: parent; anchors.margins: 8; content.textFormat: TextArea.RichText; content.text: Logger.logText }
    }
    Rectangle {
        id: footer
        anchors.left: parent.left; anchors.right: parent.right; anchors.bottom: parent.bottom; height: 32
        color: "black"
        Rectangle { width: parent.width; height: 1; color: Theme.color.mediumorange2 }
        RowLayout {
            anchors.fill: parent; anchors.leftMargin: 16; anchors.rightMargin: 16; spacing: 16
            WorkbenchLabel { text: "uFlipper " + App.version; font.pixelSize: 12; opacity: 0.6 }
            Item { Layout.fillWidth: true }
            WorkbenchButton { text: shell.showLogs ? qsTr("Logs schließen") : qsTr("Logs"); navigation: true; implicitHeight: 32; onClicked: { shell.showLogs = !shell.showLogs; Logger.errorCount = 0 } }
            WorkbenchButton { text: qsTr("Logordner öffnen"); navigation: true; implicitHeight: 32; onClicked: Qt.openUrlExternally(Logger.logsPath) }
        }
    }
    MessageDialog { id: blocked; parent: shell; radius: 0 }
    Connections { target: Backend; function onOperationBlocked(reason) { blocked.openWithMessage(null, {title: qsTr("Vollbackup erforderlich"), message: reason}) } }
    Dialog {
        id: nfcConflict; objectName: "nfcConflictDialog"; parent: shell
        anchors.centerIn: parent; modal: true; title: qsTr("NFC-Sitzung beenden?")
        contentItem: ColumnLayout {
            spacing: 16
            WorkbenchLabel { Layout.preferredWidth: 400; Layout.fillWidth: true; wrapMode: Text.Wrap; text: qsTr("Für diese Geräteansicht muss der NFC-Lesetest beendet werden. Die Aufnahme bleibt gespeichert.") }
            RowLayout {
                spacing: 16
                WorkbenchButton { objectName: "nfc-confirm-stop"; text: qsTr("Stoppen und wechseln"); onClicked: { nfcConflict.close(); Backend.nfc.stop() } }
                WorkbenchButton { objectName: "nfc-keep-reading"; text: qsTr("Weiterlesen"); onClicked: { shell.pendingPage = ""; nfcConflict.close() } }
            }
        }
        onRejected: shell.pendingPage = ""
    }
    Dialog {
        id: nfcBlocked; parent: shell; anchors.centerIn: parent; modal: true
        title: qsTr("Geräte-Stop nicht bestätigt")
        standardButtons: Dialog.Ok
        contentItem: WorkbenchLabel { width: 400; wrapMode: Text.Wrap; text: qsTr("Bitte den Flipper neu verbinden, bevor eine andere Geräteaktion beginnt. Die unterbrochene Aufnahme bleibt erhalten.") }
    }
    Connections {
        target: Backend.nfc
        function onChanged() {
            if (!Backend.nfc.active && shell.pendingPage.length > 0) {
                if (Backend.nfc.stopConfirmed && !Backend.nfc.blocked) shell.currentPage = shell.pendingPage
                shell.pendingPage = ""
            }
            if (!Backend.nfc.active && shell.quitAfterNfcStop) shell.Window.window.close()
        }
    }
    Dialog {
        id: backupConflict; objectName: "backupConflictDialog"; parent: shell
        anchors.centerIn: parent; modal: true; title: qsTr("Sicherung beenden?")
        contentItem: ColumnLayout {
            spacing: 16
            WorkbenchLabel { Layout.preferredWidth: 400; Layout.fillWidth: true; wrapMode: Text.Wrap; text: Backend.backup.canCancel ? qsTr("Diese Geräteansicht benötigt denselben Zugriff. Beim Abbrechen bleibt die unvollständige Sicherung erhalten. Danach den Flipper kurz neu verbinden.") : qsTr("Die Geräteverbindung ist noch belegt oder die lokale Prüfung läuft. Bitte den Abschluss abwarten oder den Flipper neu verbinden.") }
            RowLayout {
                spacing: 16
                WorkbenchButton { objectName: "backup-confirm-stop"; text: qsTr("Sicherung abbrechen"); enabled: Backend.backup.canCancel; onClicked: { backupConflict.close(); Backend.backup.cancel(); shell.currentPage = "backups" } }
                WorkbenchButton { objectName: "backup-keep-copying"; text: qsTr("Weiter sichern"); onClicked: backupConflict.close() }
            }
        }
    }
    Connections {
        target: Backend.backup
        function onChanged() { if (!Backend.backup.active && shell.quitAfterBackupStop) shell.Window.window.close() }
    }
    Component { id: nfcComponent; NfcPage {} }
    Component { id: dashboardComponent; DashboardPage {} }
    Component { id: settingsComponent; SettingsPage {} }
    Component { id: screenComponent; ScreenPage {} }
    Component { id: filesComponent; FilesPage {} }
    Component { id: backupsComponent; CatalogPage {} }
    Component { id: toolComponent; ToolPage { toolKey: shell.currentPage } }
    Component {
        id: firmwareComponent
        Item {
            WorkbenchGrid { anchors.fill: parent }
            ColumnLayout {
                anchors.fill: parent; anchors.margins: 24; spacing: 16
                WorkbenchTitle { title: qsTr("Firmware"); subtitle: qsTr("Bestehende Gerätepflege. Die erweiterbare Firmware-Bibliothek folgt.") }
                WorkbenchHint { text: qsTr("Firmwareänderungen bleiben angehalten, bis ein geprüftes Vollbackup erstellt werden kann.") }
                Item {
                    Layout.fillWidth: true; Layout.fillHeight: true
                    Item {
                        anchors.centerIn: parent; width: 800; height: 394
                        scale: Math.min(1, (parent.width - 8) / width, (parent.height - 8) / height)
                        HomeOverlay { anchors.fill: parent; backgroundRect: shellBg; opacity: Backend.backendState === ApplicationBackend.Ready ? 1 : 0 }
                        DeviceWidget { x: 400; y: 82; visible: Backend.backendState === ApplicationBackend.Ready; onScreenStreamRequested: shell.currentPage = "screen" }
                        NoDeviceOverlay { anchors.fill: parent; opacity: Backend.backendState === ApplicationBackend.WaitingForDevices ? 1 : 0 }
                    }
                }
            }
        }
    }
    Shortcut { sequence: "Ctrl+1"; onActivated: if (!shell.maintenance) shell.currentPage = "dashboard" }
    Shortcut { sequence: "Ctrl+2"; onActivated: if (!shell.maintenance) shell.currentPage = "infrared" }
    Shortcut { sequence: StandardKey.Preferences; onActivated: if (!shell.maintenance) shell.currentPage = "settings" }
}
