import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15
import Qt5Compat.GraphicalEffects

Item {
    FontLoader { id: solidFont; source: "qrc:/QML/fonts/fa-solid-900.ttf" }

    // ========== 辅助函数：安全显示数值/字符串 ==========
    function safeNum(val, unit = "", decimal = 1) {
        if (val !== undefined && val !== null) {
            if (typeof val === "number") {
                return val.toFixed(decimal) + unit;
            }
            return val + unit;
        }
        return "--" + unit;
    }
    function safeInt(val, unit = "") {
        if (val !== undefined && val !== null) {
            if (typeof val === "number") {
                return Math.floor(val) + unit;
            }
            return val + unit;
        }
        return "--" + unit;
    }
    function safeStr(val) {
        return (val !== undefined && val !== null && val !== "") ? val : "--";
    }

    // ========== 动态数据属性（默认有模拟值） ==========
    property var batteryLevel: 75
    property var batteryRange: 450
    property var avgConsumption: 12.5
    property var energyRecovery: 5

    property var tireFrontLeft: 2.4
    property var tireFrontRight: 2.4
    property var tireRearLeft: 2.4
    property var tireRearRight: 2.4

    property var tripA: 125.6
    property var tripB: 452.3
    property var totalMileage: 12580
    property var avgSpeed: 45

    property string driveMode: "舒适"
    property string gearPosition: "D"

    ColumnLayout {
        anchors.fill: parent
        spacing: 20

        RowLayout {
            Text {
                text: " 行车电脑"
                font.family: solidFont.name
                font.pixelSize: 26
                font.weight: Font.Bold
                color: "#0c1c32"
            }
        }

        GridLayout {
            columns: 2
            columnSpacing: 20
            rowSpacing: 20
            Layout.fillWidth: true
            Layout.fillHeight: true

            // 能耗与续航卡片
            Rectangle {
                id: cardEnergy
                Layout.fillWidth: true
                Layout.fillHeight: true
                color: "#f6faff"
                radius: 28
                border.color: "#d0ddee"
                border.width: 1
                property bool hovered: false
                layer.enabled: true
                layer.effect: DropShadow {
                    transparentBorder: true
                    color: "#10000000"
                    radius: 12
                    samples: 25
                    horizontalOffset: 0
                    verticalOffset: 3
                }
                HoverHandler { onHoveredChanged: cardEnergy.hovered = hovered }

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 22
                    spacing: 16

                    RowLayout {
                        Text {
                            text: ""
                            font.family: solidFont.name
                            font.pixelSize: 22
                            color: "#2670e8"
                        }
                        Text {
                            text: "能耗与续航"
                            font.pixelSize: 18
                            font.weight: Font.Bold
                            color: "#0b1d33"
                        }
                    }

                    GridLayout {
                        columns: 2
                        columnSpacing: 16
                        rowSpacing: 12
                        Text { text: "剩余电量"; font.pixelSize: 15; color: "#1f3853" }
                        Text {
                            text: safeInt(batteryLevel, "%")
                            font.pixelSize: 15
                            font.weight: Font.Bold
                            color: "#0b1d30"
                        }
                        Text { text: "续航里程"; font.pixelSize: 15; color: "#1f3853" }
                        Text {
                            text: safeInt(batteryRange, " km")
                            font.pixelSize: 15
                            font.weight: Font.Bold
                            color: "#0b1d30"
                        }
                        Text { text: "平均能耗"; font.pixelSize: 15; color: "#1f3853" }
                        Text {
                            text: safeNum(avgConsumption, " kWh/100km", 1)
                            font.pixelSize: 15
                            font.weight: Font.Bold
                            color: "#0b1d30"
                        }
                        Text { text: "能量回收"; font.pixelSize: 15; color: "#1f3853" }
                        Text {
                            text: safeInt(energyRecovery, " kW")
                            font.pixelSize: 15
                            font.weight: Font.Bold
                            color: "#0b1d30"
                        }
                    }

                    Rectangle {
                        Layout.fillWidth: true
                        height: 8
                        color: "#dde5ef"
                        radius: 4
                        Rectangle {
                            width: parent.width * ((batteryLevel !== undefined && batteryLevel !== null) ? batteryLevel / 100 : 0)
                            height: parent.height
                            color: "#27a068"
                            radius: 4
                        }
                    }
                }
            }

            // 胎压监测卡片
            Rectangle {
                id: cardTire
                Layout.fillWidth: true
                Layout.fillHeight: true
                color: "#f6faff"
                radius: 28
                border.color: "#d0ddee"
                border.width: 1
                property bool hovered: false
                layer.enabled: true
                layer.effect: DropShadow {
                    transparentBorder: true
                    color: "#10000000"
                    radius: 12
                    samples: 25
                    horizontalOffset: 0
                    verticalOffset: 3
                }
                HoverHandler { onHoveredChanged: cardTire.hovered = hovered }

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 22
                    spacing: 16

                    RowLayout {
                        Text {
                            text: "胎压监测"
                            font.pixelSize: 18
                            font.weight: Font.Bold
                            color: "#0b1d33"
                        }
                    }

                    GridLayout {
                        columns: 2
                        columnSpacing: 20
                        rowSpacing: 16

                        // 左前
                        Rectangle {
                            width: 100
                            height: 70
                            color: "white"
                            radius: 18
                            border.color: "#cddef2"
                            border.width: 1
                            ColumnLayout {
                                anchors.centerIn: parent
                                spacing: 4
                                Text {
                                    text: "左前"
                                    font.pixelSize: 13
                                    color: "#5b728f"
                                    Layout.alignment: Qt.AlignHCenter
                                }
                                Text {
                                    text: safeNum(tireFrontLeft, " bar", 1)
                                    font.pixelSize: 16
                                    font.weight: Font.Bold
                                    color: "#0b1d30"
                                    Layout.alignment: Qt.AlignHCenter
                                }
                            }
                        }
                        // 右前
                        Rectangle {
                            width: 100
                            height: 70
                            color: "white"
                            radius: 18
                            border.color: "#cddef2"
                            border.width: 1
                            ColumnLayout {
                                anchors.centerIn: parent
                                spacing: 4
                                Text {
                                    text: "右前"
                                    font.pixelSize: 13
                                    color: "#5b728f"
                                    Layout.alignment: Qt.AlignHCenter
                                }
                                Text {
                                    text: safeNum(tireFrontRight, " bar", 1)
                                    font.pixelSize: 16
                                    font.weight: Font.Bold
                                    color: "#0b1d30"
                                    Layout.alignment: Qt.AlignHCenter
                                }
                            }
                        }
                        // 左后
                        Rectangle {
                            width: 100
                            height: 70
                            color: "white"
                            radius: 18
                            border.color: "#cddef2"
                            border.width: 1
                            ColumnLayout {
                                anchors.centerIn: parent
                                spacing: 4
                                Text {
                                    text: "左后"
                                    font.pixelSize: 13
                                    color: "#5b728f"
                                    Layout.alignment: Qt.AlignHCenter
                                }
                                Text {
                                    text: safeNum(tireRearLeft, " bar", 1)
                                    font.pixelSize: 16
                                    font.weight: Font.Bold
                                    color: "#0b1d30"
                                    Layout.alignment: Qt.AlignHCenter
                                }
                            }
                        }
                        // 右后
                        Rectangle {
                            width: 100
                            height: 70
                            color: "white"
                            radius: 18
                            border.color: "#cddef2"
                            border.width: 1
                            ColumnLayout {
                                anchors.centerIn: parent
                                spacing: 4
                                Text {
                                    text: "右后"
                                    font.pixelSize: 13
                                    color: "#5b728f"
                                    Layout.alignment: Qt.AlignHCenter
                                }
                                Text {
                                    text: safeNum(tireRearRight, " bar", 1)
                                    font.pixelSize: 16
                                    font.weight: Font.Bold
                                    color: "#0b1d30"
                                    Layout.alignment: Qt.AlignHCenter
                                }
                            }
                        }
                    }
                }
            }

            // 行程数据卡片
            Rectangle {
                id: cardTrip
                Layout.fillWidth: true
                Layout.fillHeight: true
                color: "#f6faff"
                radius: 28
                border.color: "#d0ddee"
                border.width: 1
                property bool hovered: false
                layer.enabled: true
                layer.effect: DropShadow {
                    transparentBorder: true
                    color: "#10000000"
                    radius: 12
                    samples: 25
                    horizontalOffset: 0
                    verticalOffset: 3
                }
                HoverHandler { onHoveredChanged: cardTrip.hovered = hovered }

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 22
                    spacing: 16

                    RowLayout {
                        Text {
                            text: "行程数据"
                            font.pixelSize: 18
                            font.weight: Font.Bold
                            color: "#0b1d33"
                        }
                    }

                    GridLayout {
                        columns: 2
                        columnSpacing: 16
                        rowSpacing: 12
                        Text { text: "小计里程 A"; font.pixelSize: 15; color: "#1f3853" }
                        Text {
                            text: safeNum(tripA, " km", 1)
                            font.pixelSize: 15
                            font.weight: Font.Bold
                            color: "#0b1d30"
                        }
                        Text { text: "小计里程 B"; font.pixelSize: 15; color: "#1f3853" }
                        Text {
                            text: safeNum(tripB, " km", 1)
                            font.pixelSize: 15
                            font.weight: Font.Bold
                            color: "#0b1d30"
                        }
                        Text { text: "总里程"; font.pixelSize: 15; color: "#1f3853" }
                        Text {
                            text: safeInt(totalMileage, " km")
                            font.pixelSize: 15
                            font.weight: Font.Bold
                            color: "#0b1d30"
                        }
                        Text { text: "平均速度"; font.pixelSize: 15; color: "#1f3853" }
                        Text {
                            text: safeInt(avgSpeed, " km/h")
                            font.pixelSize: 15
                            font.weight: Font.Bold
                            color: "#0b1d30"
                        }
                    }
                }
            }

            // 驾驶模式卡片
            Rectangle {
                id: cardMode
                Layout.fillWidth: true
                Layout.fillHeight: true
                color: "#f6faff"
                radius: 28
                border.color: "#d0ddee"
                border.width: 1
                property bool hovered: false
                layer.enabled: true
                layer.effect: DropShadow {
                    transparentBorder: true
                    color: "#10000000"
                    radius: 12
                    samples: 25
                    horizontalOffset: 0
                    verticalOffset: 3
                }
                HoverHandler { onHoveredChanged: cardMode.hovered = hovered }

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 22
                    spacing: 16

                    RowLayout {
                        Text {
                            text: ""
                            font.family: solidFont.name
                            font.pixelSize: 22
                            color: "#2670e8"
                        }
                        Text {
                            text: "驾驶模式"
                            font.pixelSize: 18
                            font.weight: Font.Bold
                            color: "#0b1d33"
                        }
                    }

                    RowLayout {
                        spacing: 10
                        Repeater {
                            model: ["舒适", "运动", "节能"]
                            Rectangle {
                                Layout.fillWidth: true
                                height: 40
                                radius: 20
                                color: (driveMode === modelData) ? "#2670e8" : "#f0f6ff"
                                border.color: (driveMode === modelData) ? "#2670e8" : "#cfe0f5"
                                border.width: 1
                                Text {
                                    anchors.centerIn: parent
                                    text: modelData
                                    font.pixelSize: 14
                                    font.weight: Font.Bold
                                    color: (driveMode === modelData) ? "white" : "#1b3a5a"
                                }
                            }
                        }
                    }

                    RowLayout {
                        Text { text: "当前挡位"; font.pixelSize: 15; color: "#1f3853" }
                        Item { Layout.fillWidth: true }
                        Label {
                            text: gearPosition
                            font.pixelSize: 26
                            font.weight: Font.Bold
                            color: "#0b1d30"
                            background: Rectangle {
                                color: "white"
                                radius: 30
                                border.color: "#c1d6f0"
                                border.width: 1
                                implicitWidth: 50
                                implicitHeight: 40
                            }
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }
                    }
                }
            }
        }
    }
}