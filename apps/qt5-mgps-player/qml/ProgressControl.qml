import QtQuick 2.12
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4

Rectangle {
    id: root
    color: "transparent"
    height: current.height

    Rectangle {
        id: background
        color: "black"
        opacity: .3
        anchors.fill: parent
    }

    Text {
        id: current
        text: currentDrive.playbackString
        color: "white"
        padding: 10

        anchors {
            left: parent.left
            verticalCenter: parent.verticalCenter
        }
    }

    Text {
        id: duration
        text: currentDrive.durationString
        color: "white"
        padding: 10

        anchors {
            right: parent.right
            verticalCenter: parent.verticalCenter
        }
    }

    ProgressBar {
        value: currentDrive.duration > 0 ? currentDrive.playback / currentDrive.duration : 0;
        anchors {
            left: current.right
            right: duration.left
            verticalCenter: parent.verticalCenter
        }
        style: ProgressBarStyle {
            background: Rectangle {
                color: "white"
                opacity: .5
                radius: 0
                border.width: 0
                implicitWidth: 200
                implicitHeight: 3
            }
            progress: Rectangle {
                color: "red"
                radius: 0
            }
        }
    }
}
