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

        onPositionChanged: currentTrip.playback = position
        onError: console.error(error, errorString)
    }

    focus: true
    Keys.onLeftPressed: mediaPlayer.seek(mediaPlayer.position - 5000)
    Keys.onRightPressed: mediaPlayer.seek(mediaPlayer.position + 5000)

    Component.onCompleted: currentTrip.playerAvailable(mediaPlayer)
}
