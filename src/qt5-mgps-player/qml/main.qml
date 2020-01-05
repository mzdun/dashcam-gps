import QtQuick 2.12
import QtQuick.Window 2.12
import QtLocation 5.12
import QtPositioning 5.12
import mGPS 1.0

AppWindow {
    visible: true
    width: 512
    height: 864
    iconSource: ":/img/icon.png"

    title: qsTr("Dashcam GPS Viewer") + " | " +
           trip.playbackString + " » " +
           trip.durationString + " | " +
           trip.timelineString + " | " +
           trip.carPosition

    Connections {
        id: scaler
        property size frame
        frame.width: 16
        frame.height: 9
        target: player

        onFrameRectChanged: {
            var w = player.frameRect.width
            var h = player.frameRect.height
            if (!w || !h) {
                w = 16
                h = 9
            }
            if (w !== frame.width || h !== frame.height) {
                frame.width = w
                frame.height = h
            }
        }
    }

    RouteViewer {
        id: map
        anchors {
            left: parent.left
            right: parent.right
            top: parent.top
            bottom: player.top
        }
    }

    VideoPlayer {
        id: player
        property int calculatedHight: parent.width / scaler.frame.width * scaler.frame.height
        height: calculatedHight > parent.height / 2 ? parent.height / 2 : calculatedHight
        anchors {
            left: parent.left
            right: parent.right
            bottom: parent.bottom
        }
    }
}
