import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15
import Qt5Compat.GraphicalEffects

Item {
    id: root
    height: isExpanded ? expandedHeight : collapsedHeight
    
    Behavior on height {
        NumberAnimation {
            duration: 300
            easing.type: Easing.OutCubic
        }
    }

    property real interiorTemp: 25.0
    property real targetTemp: 22.0
    property string interiorTempDisplay: "25°C"
    property string targetTempDisplay: "22°C"
    property string fanStatus: "已关闭"
    property int fanLevel: 0
    property bool fanActive: false
    property bool fanExpanded: false
    
    property real currentInteriorTemp: 25.0
    
    onTargetTempChanged: startTempTransition()
    onInteriorTempChanged: startTempTransition()
    
    function startTempTransition() {
        tempTransitionTimer.start()
    }
    
    Timer {
        id: tempTransitionTimer
        interval: 500
        repeat: true
        onTriggered: {
            var tempDiff = root.interiorTemp - root.currentInteriorTemp
            if (Math.abs(tempDiff) > 0.005) {
                var changeAmount = 0.01
                if (tempDiff > 0) {
                    root.currentInteriorTemp += changeAmount
                } else {
                    root.currentInteriorTemp -= changeAmount
                }
                root.interiorTempDisplay = root.currentInteriorTemp.toFixed(1) + "°C"
            } else {
                root.currentInteriorTemp = root.interiorTemp
                root.interiorTempDisplay = root.interiorTemp.toFixed(1) + "°C"
                stop()
            }
        }
    }
    
    onFanActiveChanged: {
        if (fanActive) {
            blueFill.width = fanButton.width
        } else {
            blueFill.width = 0
        }
    }

    FontLoader { id: solidFont; source: "qrc:/QML/fonts/fa-solid-900.ttf" }

    property bool isExpanded: true
    property real expandedHeight: 80
    property real collapsedHeight: 40
    property int autoHideDelay: 5000

    function resetAutoHideTimer() {
        autoHideTimer.restart()
    }

    function toggleExpand() {
        isExpanded = !isExpanded
        if (isExpanded) {
            resetAutoHideTimer()
        }
    }

    Timer {
        id: autoHideTimer
        interval: root.autoHideDelay
        onTriggered: {
            if (isExpanded) {
                isExpanded = false
            }
        }
    }

    Rectangle {
        id: mainContainer
        anchors.fill: parent
        color: "#ffffff"
        radius: 48
        border.color: "#d7e5fa"
        border.width: 1
        opacity: 0.85
        layer.enabled: true
        layer.effect: DropShadow {
            transparentBorder: true
            color: "#20000000"
            radius: 20
            samples: 41
            horizontalOffset: 0
            verticalOffset: 5
        }

        MouseArea {
            anchors.fill: parent
            hoverEnabled: true
            onEntered: resetAutoHideTimer()
            onPressed: resetAutoHideTimer()
        }

        RowLayout {
            id: contentRow
            anchors.fill: parent
            anchors.leftMargin: 32
            anchors.rightMargin: 32
            spacing: 20
            visible: isExpanded

            RowLayout {
                spacing: 24
                RowLayout {
                    spacing: 8
                    Text {
                        text: ""
                        font.family: solidFont.name
                        font.pixelSize: 16
                        color: "#2670e8"
                    }
                    ColumnLayout {
                        spacing: 2
                        Text {
                            text: "车内"
                            font.pixelSize: 10
                            color: "#6a8bb0"
                        }
                        Text {
                            text: root.interiorTempDisplay
                            font.pixelSize: 18
                            font.weight: Font.Bold
                            color: "#0a1e32"
                        }
                    }
                }
                Rectangle {
                    width: 1; height: 40; color: "#c7daf2"
                }
                RowLayout {
                    spacing: 8
                    Text {
                        text: ""
                        font.family: solidFont.name
                        font.pixelSize: 16
                        color: "#2670e8"
                    }
                    ColumnLayout {
                        spacing: 2
                        Text {
                            text: "目标"
                            font.pixelSize: 10
                            color: "#6a8bb0"
                        }
                        Text {
                            text: root.targetTempDisplay
                            font.pixelSize: 18
                            font.weight: Font.Bold
                            color: "#0a1e32"
                        }
                    }
                }
            }

            Item { Layout.fillWidth: true }

            RowLayout {
                id: rightGroup
                spacing: 8

                Item {
                    width: 100
                    height: 40
                    clip: false

                    Rectangle {
                        id: fanButton
                        anchors.fill: parent
                        color: "transparent"
                        radius: 20
                        border.color: root.fanActive ? "#2670e8" : "#c7daf2"
                        border.width: 1
                        clip: true
                        
                        Rectangle {
                            id: blueFill
                            width: 0
                            height: parent.height
                            color: "#2670e8"
                            radius: 19
                            Behavior on width {
                                NumberAnimation {
                                    duration: 1500
                                    easing.type: Easing.Linear
                                }
                            }
                        }
                        
                        RowLayout {
                            anchors.centerIn: parent
                            spacing: 8
                            z: 2
                            Text { 
                                text: ""; 
                                font.family: solidFont.name; 
                                font.pixelSize: 16; 
                                color: root.fanActive ? "white" : "#2670e8" 
                            }
                            Text { 
                                text: root.fanStatus; 
                                font.pixelSize: 14; 
                                color: root.fanActive ? "white" : "#0a1e32" 
                            }
                        }
                        
                        Timer {
                            id: longPressTimer
                            interval: 1500
                            onTriggered: {
                                if (!root.fanActive) {
                                    root.fanActive = true
                                    root.fanStatus = "自动 · 1档"
                                    root.fanLevel = 1
                                } else {
                                    root.fanActive = false
                                    root.fanStatus = "已关闭"
                                    root.fanLevel = 0
                                    root.fanExpanded = false
                                }
                            }
                        }
                        
                        MouseArea {
                            anchors.fill: parent
                            hoverEnabled: true
                            onEntered: resetAutoHideTimer()
                            onPressed: {
                                resetAutoHideTimer()
                                longPressTimer.start()
                                if (!root.fanActive) {
                                    blueFill.width = parent.width
                                } else {
                                    blueFill.width = 0
                                }
                            }
                            onReleased: {
                                longPressTimer.stop()
                                if (!root.fanActive) {
                                    blueFill.width = 0
                                } else {
                                    blueFill.width = parent.width
                                }
                            }
                        }
                    }

                    Rectangle {
                        id: expandedList
                        width: parent.width
                        height: root.fanExpanded ? (36 * 5 + 16) : 0
                        anchors.bottom: fanButton.top
                        anchors.bottomMargin: 4
                        color: "#f2f9ff"
                        radius: 20
                        border.color: "#cde0f8"
                        border.width: 1
                        clip: true
                        
                        Behavior on height {
                            NumberAnimation {
                                duration: 300
                                easing.type: Easing.OutCubic
                            }
                        }
                        
                        ColumnLayout {
                            anchors.fill: parent
                            anchors.margins: 4
                            spacing: 4
                            
                            Repeater {
                                model: ["自动", "1", "2", "3", "4"]
                                
                                Rectangle {
                                    height: 36
                                    width: parent.width
                                    color: root.fanLevel === index ? "#2670e8" : "transparent"
                                    radius: 18
                                    border.color: root.fanLevel === index ? "#2670e8" : "#cde0f8"
                                    border.width: 1
                                    RowLayout {
                                        anchors.centerIn: parent
                                        spacing: 8
                                        Text { 
                                            text: modelData; 
                                            font.pixelSize: 13; 
                                            color: root.fanLevel === index ? "white" : "#0a1e32" 
                                        }
                                    }
                                    MouseArea {
                                        anchors.fill: parent
                                        onClicked: {
                                            root.fanLevel = index
                                            root.fanStatus = index === 0 ? "自动 · 1档" : ("手动 · " + index + "档")
                                            root.fanExpanded = false
                                            resetAutoHideTimer()
                                        }
                                    }
                                }
                            }
                        }
                    }
                }

                Rectangle {
                    width: 16
                    height: 16
                    radius: 8
                    color: root.fanActive ? "#2ecc71" : "#c7daf2"
                    border.color: root.fanActive ? "#27ae60" : "#b0c4de"
                    border.width: 1
                    Behavior on color {
                        ColorAnimation {
                            duration: 300
                        }
                    }
                    
                    MouseArea {
                        anchors.fill: parent
                        hoverEnabled: true
                        onEntered: resetAutoHideTimer()
                        onClicked: {
                            if (root.fanActive) {
                                root.fanExpanded = !root.fanExpanded
                                resetAutoHideTimer()
                            }
                        }
                    }
                }
            }
        }

        Rectangle {
            id: pullHandle
            anchors.centerIn: parent
            width: 60
            height: 6
            color: "#c7daf2"
            radius: 3
            visible: !isExpanded

            MouseArea {
                anchors.fill: parent
                anchors.leftMargin: -20
                anchors.rightMargin: -20
                anchors.topMargin: -10
                anchors.bottomMargin: -10
                hoverEnabled: true
                cursorShape: Qt.PointingHandCursor
                onClicked: toggleExpand()
            }
        }
    }

    Component.onCompleted: {
        console.log("[BottomControls] 初始化完成")
        autoHideTimer.start()
    }
}
