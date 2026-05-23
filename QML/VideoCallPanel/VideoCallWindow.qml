import QtQuick 2.15
import QtQuick.Window 2.15
import VideoRenderer 1.0

Window {
    id: videoCallWindow
    visible: false
    width: 800
    height: 600
    minimumWidth: 640
    minimumHeight: 480
    title: "音视频通话"
    color: "#000000"
    
    VideoRenderer {
        id: videoRenderer
        anchors.fill: parent
        url: "http://localhost:3000/stream-h264.ts"
    }
    
    Timer {
        interval: 100
        running: true
        repeat: false
        onTriggered: {
            console.log("[VideoCallWindow] 定时器触发，开始播放")
            videoRenderer.start()
        }
    }
}
