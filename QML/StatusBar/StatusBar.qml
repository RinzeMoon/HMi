import QtQuick 2.15
import QtQuick.Layouts 1.15

Item {
    id: root
    height: 60

    // ========== 新增属性 ==========
    property string timeText: timeService ? timeService.getCurrentTime() : "9:41"
    property string roadText: "城市道路"
    property int batteryLevel: 68
    property int batteryRange: 328
    property bool bluetoothConnected: true   // 控制蓝牙图标显示状态
    property bool wifiConnected: true         // 控制Wi-Fi图标显示状态

    // 颜色属性（可自定义）
    property color primaryTextColor: "#0b1a30"
    property color secondaryTextColor: "#2d3a4f"
    property color iconColor: "#5a6f8a"
    property color batteryIconColor: "#27a068"
    property color batteryTextColor: "#1e2b3d"
    property color batteryRangeColor: "#5b728f"

    FontLoader { id: solidFont; source: "qrc:/QML/fonts/fa-solid-900.ttf" }
    FontLoader { id: brandsFont; source: "qrc:/QML/fonts/fa-brands-400.ttf" }

    Connections {
        target: timeService
        function onTimeUpdated(time, date) {
            root.timeText = time
        }
    }

    RowLayout {
        anchors.fill: parent
        spacing: 24

        RowLayout {
            spacing: 24
            Text {
                text: root.timeText
                font.pixelSize: 18
                font.weight: Font.DemiBold
                color: root.primaryTextColor
            }
            Row {
                spacing: 8
                Text {
                    text: ""  // 位置图标
                    font.family: solidFont.name
                    font.pixelSize: 16
                    color: root.iconColor
                }
                Text {
                    text: root.roadText
                    font.pixelSize: 15
                    color: root.secondaryTextColor
                }
            }
        }

        Item { Layout.fillWidth: true }

        RowLayout {
            spacing: 20
            Text {
                text: ""  // 蓝牙图标
                font.family: brandsFont.name
                font.pixelSize: 16
                color: root.bluetoothConnected ? root.iconColor : "#cccccc"  // 未连接时变灰
            }
            Text {
                text: ""  // Wi-Fi图标
                font.family: solidFont.name
                font.pixelSize: 16
                color: root.wifiConnected ? root.iconColor : "#cccccc"
            }

            Rectangle {
                height: 30
                implicitWidth: 120
                color: "#eef2f8"
                radius: 20
                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 10
                    anchors.rightMargin: 10
                    spacing: 4
                    Text {
                        text: ""  // 电池图标
                        font.family: solidFont.name
                        font.pixelSize: 16
                        color: root.batteryIconColor
                    }
                    Text {
                        text: root.batteryLevel + "%"
                        font.pixelSize: 14
                        font.weight: Font.Medium
                        color: root.batteryTextColor
                    }
                    Rectangle {
                        width: 1; height: 16; color: "#cbd6e6"
                    }
                    Text {
                        text: root.batteryRange + "km"
                        font.pixelSize: 14
                        color: root.batteryRangeColor
                    }
                }
            }
        }
    }
}