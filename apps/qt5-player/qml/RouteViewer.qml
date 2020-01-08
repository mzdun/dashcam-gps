import QtQuick 2.0
import QtLocation 5.6
import QtPositioning 5.6
import mGPS 1.0

Rectangle {
    id: root
    property variant lines: []

    Plugin {
        id: mapService
        name: "osm"
    }

    Map {
        id: map
        anchors.fill: parent
        plugin: mapService
        visibleRegion: currentDrive.visibleRegion

        MapQuickItem {
            visible: true
            id: carMarker
            anchorPoint.x: image.width/2
            anchorPoint.y: image.height
            sourceItem: Image {
                id: image
                source: "/img/marker.svg"
            }
            coordinate: currentDrive.carPosition
        }
    }

    Connections {
        target: currentDrive
        onDriveChanged: updateLines()
    }

    Component.onCompleted: updateLines()

    function removeLines() {
        for (let line_ndx in root.lines) {
            map.removeMapItem(lines[line_ndx])
            root.lines[line_ndx].destroy()
        }
        lines = []
    }

    function updateLines() {
        removeLines()
        var lines = currentDrive.lines
        for (let line_ndx in lines) {
            var line = lines[line_ndx]

            var outline = Qt.createQmlObject('import QtLocation 5.6; MapPolyline { line.width: 7; line.color: "white" }', map, "RouteViewer-updateLines")
            outline.path = line.path
            root.lines.push(outline)
            map.addMapItem(outline)

            var plot = Qt.createQmlObject('import QtLocation 5.6; MapPolyline { line.width: 3; line.color: "#0085ca" }', map, "RouteViewer-updateLines")
            plot.path = line.path
            root.lines.push(plot)
            map.addMapItem(plot)
        }
        map.removeMapItem(carMarker)
        map.addMapItem(carMarker)
    }
}
