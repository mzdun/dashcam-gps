import QtQuick 2.0
import QtLocation 5.6
import QtPositioning 5.6
import mGPS 1.0

Rectangle {
    id: root
    property variant segments: []

    Plugin {
        id: mapService
        name: "osm"
    }

    Map {
        id: map
        anchors.fill: parent
        plugin: mapService
        visibleRegion: trip.visibleRegion

        MapQuickItem {
            visible: true
            id: carMarker
            anchorPoint.x: image.width/2
            anchorPoint.y: image.height
            sourceItem: Image {
                id: image
                source: "/img/marker.svg"
            }
            coordinate: trip.carPosition
        }
    }

    Connections {
        target: trip
        onSegmentsChanged: updateSegments()
    }

    Component.onCompleted: updateSegments()

    function removeSegments() {
        for (let seg_ndx in segments) {
            map.removeMapItem(segments[seg_ndx])
            segments[seg_ndx].destroy()
        }
        segments = []
    }

    function updateSegments() {
        removeSegments()
        var segs = trip.segments
        for (let seg_ndx in segs) {
            var seg = segs[seg_ndx]

            var outline = Qt.createQmlObject('import QtLocation 5.6; MapPolyline { line.width: 7; line.color: "white" }', map, "RouteViewer-updateSegments")
            outline.path = seg.path
            segments.push(outline)
            map.addMapItem(outline)

            var plot = Qt.createQmlObject('import QtLocation 5.6; MapPolyline { line.width: 3; line.color: "#0085ca" }', map, "RouteViewer-updateSegments")
            plot.path = seg.path
            segments.push(plot)
            map.addMapItem(plot)
        }
        map.removeMapItem(carMarker)
        map.addMapItem(carMarker)
    }
}
