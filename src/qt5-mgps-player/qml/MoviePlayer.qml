import QtQuick 2.0

Rectangle {
    color: "black"

    Rectangle {
        color: "silver"
        width: height / 9 * 16
        anchors {
            top: parent.top
            bottom: parent.bottom
            horizontalCenter: parent.horizontalCenter
        }
    }
}
