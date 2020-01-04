import QtQuick 2.0
import QtLocation 5.6
import QtPositioning 5.6
import mGPS 1.0

Rectangle {
    id: root

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
