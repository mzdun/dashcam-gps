import QtQuick 2.12

Rectangle {
    height: text.height + kmph.height
    width: Math.max(text.width, kmph.width)

    Rectangle {
        id: background
        color: "#444444"
        anchors.fill: parent
    }

    Text {
        id: text
        text: currentTrip.carSpeed
        color: "white"
        font.bold: true
        font.pointSize: 14
        padding: 10
        bottomPadding: 6
        anchors {
            top: parent.top
            horizontalCenter: parent.horizontalCenter
        }
    }

    Text {
        id: kmph
        text: "KM / H"
        color: "white"
        font.bold: true
        font.pointSize: 8
        padding: 10
        topPadding: 0
        anchors {
            top: text.bottom
            horizontalCenter: parent.horizontalCenter
        }
    }
}
