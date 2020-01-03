import QtQuick 2.12
import QtQuick.Window 2.12
import QtLocation 5.6
import QtPositioning 5.6

Window {
    visible: true
    width: 722
    height: 729
    title: qsTr("Dashcam GPS Viewer") + " | " +
           trip.playbackString + " » " +
           trip.durationString + " | " +
           trip.timelineString + " | " +
           trip.position

    Plugin {
        id: mapService
        name: "osm"
    }

    Map {
        id: map
        anchors.fill: parent
        plugin: mapService
        visibleRegion: trip.visibleRegion
    }
}
