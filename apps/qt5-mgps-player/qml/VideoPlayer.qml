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

        onPositionChanged: currentDrive.playback = position
        onError: console.error(error, errorString)
    }

    Component.onCompleted: currentDrive.playerAvailable(mediaPlayer)
}
