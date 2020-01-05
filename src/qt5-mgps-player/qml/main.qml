import QtQuick 2.12
import QtQuick.Window 2.12
import QtLocation 5.6
import QtPositioning 5.6
import mGPS 1.0

AppWindow {
    visible: true
    width: 722
    height: 729
    iconSource: ":/img/icon.png"
    title: qsTr("Dashcam GPS Viewer") + " | " +
           trip.playbackString + " » " +
           trip.durationString + " | " +
           trip.timelineString + " | " +
           trip.carPosition

    RouteViewer {
        id: map
        anchors {
            left: parent.left
            right: parent.right
            top: parent.top
            bottom: player.top
        }
    }

    MoviePlayer {
        id: player
        property int calculatedHight: parent.width / 16 * 9
        height: calculatedHight > parent.height / 3 ? parent.height / 3 : calculatedHight
        anchors {
            left: parent.left
            right: parent.right
            bottom: parent.bottom
        }
    }
}
