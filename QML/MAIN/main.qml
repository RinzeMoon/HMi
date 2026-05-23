import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.15

// 显式导入组件路径（您要求保留）
import "qrc:/QML/HMIBackground"
import "qrc:/QML/DashboardPanel"
import "qrc:/QML/MapPanel"
import "qrc:/QML/CarPanel"
import "qrc:/QML/MusicPanel"   // 新增音乐面板导入
import "qrc:/QML/VideoCallPanel"

Window {
    visible: true
    width: 1200
    height: 780
    title: "汽车HMI"
    color: "transparent"
    x: (Screen.width - width) / 2
    y: (Screen.height - height) / 2

    // 加载字体
    FontLoader { id: solidFont; source: "qrc:/QML/fonts/fa-solid-900.ttf" }
    FontLoader { id: regularFont; source: "qrc:/QML/fonts/fa-regular-400.ttf" }
    FontLoader { id: brandsFont; source: "qrc:/QML/fonts/fa-brands-400.ttf" }

    HMIBackground {
        id: background
        anchors.fill: parent

        // 将字体对象传递给子组件（通过属性）
        property var solidFont: solidFont
        property var regularFont: regularFont
        property var brandsFont: brandsFont

        Component.onCompleted: {
            background.activeSidebarIndex = 0
        }

        onActiveSidebarIndexChanged: {
            console.log("切换到侧边栏索引:", background.activeSidebarIndex)
        }

        onStartOverlaySignal: {
            globalOverlay.visible = true
        }

        onStopOverlaySignal: {
            globalOverlay.visible = false
        }
    }

    Rectangle {
        id: globalOverlay
        anchors.fill: parent
        color: "#000000"
        opacity: 0.5
        visible: false
        z: 100
        Component.onCompleted: {
            globalOverlay.visible = true
            globalOverlay.visible = false
        }

        Column {
            anchors.centerIn: parent
            spacing: 20
            visible: globalOverlay.visible
            z: 101

            Text {
                id: timerText
                anchors.horizontalCenter: parent.horizontalCenter
                text: background.recordingTime.toFixed(1) + " s"
                font.pixelSize: 60
                font.weight: Font.Bold
                color: "#ffffff"
            }

            Text {
                anchors.horizontalCenter: parent.horizontalCenter
                text: "正在录制中..."
                font.pixelSize: 20
                color: "#ffffff"
                opacity: 0.8
            }

            Text {
                anchors.horizontalCenter: parent.horizontalCenter
                text: "松开按钮结束"
                font.pixelSize: 14
                color: "#ffffff"
                opacity: 0.6
            }
        }
    }
    
    VideoCallWindow {
        id: videoCallWindow
    }
    
    Rectangle {
        id: videoCallButton
        width: 50
        height: 50
        x: 20
        y: 20
        radius: 25
        color: "#2670e8"
        z: 200
        
        Text {
            anchors.centerIn: parent
            text: "📹"
            font.pixelSize: 24
        }
        
        MouseArea {
            anchors.fill: parent
            onClicked: {
                videoCallWindow.visible = true
                videoCallWindow.show()
                videoCallWindow.raise()
            }
        }
    }
}