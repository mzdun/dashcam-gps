import QtQuick 2.0
import QtMultimedia 5.12

Rectangle {
    color: "black"
    property alias mediaSource: mediaPlayer.source
    property rect frameRect: videoOutput.sourceRect

    VideoOutput {
        id: videoOutput
        anchors.fill: parent
        source: mediaPlayer
    }

    MediaPlayer {
        id: mediaPlayer
        autoPlay: true
        notifyInterval: 100

        onPositionChanged: trip.playback = mediaPlayer.position + trip.clipOffset

        onError: console.log(error, errorString)
        onPaused: console.log(">>> paused")
        onPlaying: console.log(">>> playing")
        onStopped: console.log(">>> stopped")
        onDurationChanged: console.log(">>> len", duration/1000)
    }
}
