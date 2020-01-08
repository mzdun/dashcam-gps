import QtQuick 2.12
import QtQuick.Window 2.12
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.0
import mGPS 1.0

AppWindow {
    visible: true
    width: 1200
    height: 640
    iconSource: ":/img/icon.png"

    title: qsTr("Dashcam GPS Viewer")

    SplitView {
        id: split
        anchors.fill: parent
        orientation: Qt.Horizontal

        Rectangle {
            id: collections
            color: "white"
            width: 200
        }

        Rectangle {
            id: items
            color: "white"
            Layout.fillWidth: true
        }

        Viewer {
            id: viewer
            width: 400
        }

    }
}
