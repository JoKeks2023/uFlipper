import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import Primitives 1.0
import Theme 1.0
import "WorkbenchGeometry.js" as Geometry
ComboBox {
    Layout.alignment: Qt.AlignBottom
    implicitHeight: Geometry.controlHeight
    font.family: "Share Tech"
    font.pixelSize: 17
    font.capitalization: Font.MixedCase
    borderWidth: 1
    backgroundColor: ColorSet {
        normal: "#120900"; hover: "#261407"; down: Theme.color.lightorange2; disabled: "#120900"
    }
    strokeColor: ColorSet {
        normal: Theme.color.mediumorange2; hover: Theme.color.lightorange2
        down: Theme.color.lightorange1; disabled: Theme.color.mediumorange3
    }
}
