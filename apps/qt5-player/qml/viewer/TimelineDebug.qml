import QtQuick 2.12

Rectangle {
    height: debug.height
    width: debug.width

    Rectangle {
        id: background
        color: "#444444"
        anchors.fill: parent
    }

    Text {
        id: debug
        text: currentTrip.timelineString
        color: "orange"
        font.bold: true
        font.pointSize: 11
        padding: 10
        anchors {
            right: parent.right
            verticalCenter: parent.verticalCenter
        }
    }
}
