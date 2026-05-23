import QtQuick 2.15
import QtQuick.Layouts 1.15
import Qt5Compat.GraphicalEffects
import "qrc:/QML/Sidebar"
import "qrc:/QML/StatusBar"
import "qrc:/QML/BottomControls"
import "qrc:/QML/DashboardPanel"
import "qrc:/QML/MapPanel"
import "qrc:/QML/CarPanel"
import "qrc:/QML/MusicPanel"
import "qrc:/QML/AIPanel"

Item {
    id: root
    width: 1200
    height: 780

    property int activeSidebarIndex: 0

    property string globalSongName: ""
    property string globalArtistName: ""
    property string globalCurrentTime: "--:--"
    property string globalTotalTime: "--:--"
    property real globalProgressPercent: 0.0
    property bool globalIsPlaying: false
    property real recordingTime: 0.0
    property bool showRecordingOverlay: false

    signal startOverlaySignal()
    signal stopOverlaySignal()
    signal searchAndPlayMusic(string songName)
    signal songPlayed(string songName, string artistName)
    signal controlAirConditioner(bool fanActive, int leftTemp, int rightTemp, int fanLevel)
    signal searchNavigation(string startAddress, string endAddress, string city)

    Rectangle {
        anchors.fill: parent
        color: "#ffffff"
        radius: 48
        border.color: "#d7e5fa"
        border.width: 1
        layer.enabled: true
        layer.effect: DropShadow {
            transparentBorder: true
            color: "#20000000"
            radius: 30
            samples: 61
            horizontalOffset: 0
            verticalOffset: 10
        }
    }

    StatusBar {
        id: statusBar
        anchors.top: parent.top
        anchors.topMargin: 24
        anchors.left: parent.left
        anchors.leftMargin: 28
        anchors.right: parent.right
        anchors.rightMargin: 28
    }

    ColumnLayout {
        anchors.top: statusBar.bottom
        anchors.topMargin: 20
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 24
        anchors.left: parent.left
        anchors.leftMargin: 28
        anchors.right: parent.right
        anchors.rightMargin: 28
        spacing: 20

        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 20

            Sidebar {
                id: sidebar
                Layout.preferredWidth: 80
                Layout.fillHeight: true
                onSidebarClicked: function(index) {
                    root.activeSidebarIndex = index
                }
                activeIndex: root.activeSidebarIndex
            }

            Item {
                id: contentPlaceholder
                Layout.fillWidth: true
                Layout.fillHeight: true
                clip: true

                DashboardPanel {
                    id: dashboardPanel
                    anchors.fill: parent
                    visible: root.activeSidebarIndex === 0
                }

                MapPanel {
                    id: mapPanel
                    anchors.fill: parent
                    visible: root.activeSidebarIndex === 1
                }

                CarPanel {
                    id: carPanel
                    anchors.fill: parent
                    visible: root.activeSidebarIndex === 2
                    batteryRange: dashboardPanel.range
                    energyRecovery: dashboardPanel.energyRecovery
                }

                MusicPanel {
                    id: musicPanel
                    anchors.fill: parent
                    visible: root.activeSidebarIndex === 3
                    
                    onSongPlayed: function(songName, artistName) {
                        console.log("MusicPanel song played:", songName, "by", artistName)
                        agentLoop.run("请介绍一下歌曲《" + songName + "》和歌手" + artistName)
                    }
                }

                AIPanel {
                    id: aiPanel
                    anchors.fill: parent
                    visible: root.activeSidebarIndex === 4

                    onStartRecordingSignal: {
                        root.showRecordingOverlay = true
                        root.startOverlaySignal()
                    }

                    onStopRecordingSignal: {
                        root.showRecordingOverlay = false
                        root.stopOverlaySignal()
                    }
                }
            }
        }

        BottomControls {
            id: bottomControls
            Layout.fillWidth: true
        }
    }

    onSearchAndPlayMusic: function(songName) {
        console.log("收到搜索并播放音乐:", songName)
        root.activeSidebarIndex = 3
        musicPanel.searchText = songName
        // 设置为AI触发的搜索
        musicPanel.isAiTriggeredSearch = true
        musicService.searchSongs(songName)
        musicPanel.showLyrics = false
    }
    
    onControlAirConditioner: function(fanActive, leftTemp, rightTemp, fanLevel) {
        console.log("控制空调:", fanActive, leftTemp, rightTemp, fanLevel)
        bottomControls.fanActive = fanActive
        bottomControls.targetTemp = rightTemp  // 设置目标温度
        bottomControls.targetTempDisplay = rightTemp + "°C"
        bottomControls.fanLevel = fanLevel
        
        // 假设当前车内温度，模拟温度逐渐变化到目标温度
        // 实际应用中应该从传感器获取真实的车内温度
        if (bottomControls.interiorTemp === undefined) {
            bottomControls.interiorTemp = 25.0  // 初始车内温度
        }
        // 模拟车内温度向目标温度变化
        bottomControls.interiorTemp = rightTemp
        
        // 更新风扇状态文本
        if (fanActive) {
            if (fanLevel === 0) {
                bottomControls.fanStatus = "自动 · 1档"
            } else {
                bottomControls.fanStatus = "手动 · " + fanLevel + "档"
            }
        } else {
            bottomControls.fanStatus = "已关闭"
        }
    }
    
    onSearchNavigation: function(startAddress, endAddress, city) {
        console.log("收到导航请求:", startAddress, endAddress, city)
        root.activeSidebarIndex = 1
        mapPanel.openPanelAndNavigate(startAddress, endAddress, city)
    }

    Timer {
        id: syncTimer
        interval: 100
        repeat: true
        running: true
        onTriggered: {
            try {
                if (musicPanel && musicPanel !== null) {
                    root.globalSongName = musicPanel.songName || ""
                    root.globalArtistName = musicPanel.artistName || ""
                    root.globalCurrentTime = musicPanel.currentTime || "--:--"
                    root.globalTotalTime = musicPanel.totalTime || "--:--"
                    root.globalProgressPercent = musicPanel.progressPercent || 0.0
                    root.globalIsPlaying = musicPanel.isPlaying || false
                }
                if (aiPanel && aiPanel !== null) {
                    root.recordingTime = aiPanel.recordingTime || 0.0
                    root.showRecordingOverlay = aiPanel.showOverlay || false
                }
            } catch (e) {
                console.log("[HMIBackground] syncTimer error:", e)
            }
        }
    }

    // ── DataBus → UI bridge ──
    Connections {
        target: dataBus
        function onVehicleDataChanged() {
            // Update dashboard
            if (dashboardPanel) {
                dashboardPanel.speed = dataBus.speed
                dashboardPanel.range = dataBus.range
            }
            // Update climate
            if (bottomControls) {
                bottomControls.fanActive = dataBus.fanActive
                bottomControls.fanLevel = dataBus.fanLevel
                bottomControls.targetTemp = dataBus.rightTemp
                bottomControls.targetTempDisplay = dataBus.rightTemp + "°C"
                bottomControls.interiorTemp = dataBus.rightTemp
                bottomControls.fanStatus = dataBus.fanActive
                    ? (dataBus.fanLevel === 0 ? "自动 · 1档" : "手动 · " + dataBus.fanLevel + "档")
                    : "已关闭"
            }
            // Feed context to Agent
            var parts = []
            if (dataBus.speed > 0) parts.push("车速" + dataBus.speed.toFixed(0) + "km/h")
            if (dataBus.rpm > 0) parts.push("转速" + dataBus.rpm + "rpm")
            if (dataBus.gear === 3) parts.push("档位D")
            else if (dataBus.gear === 2) parts.push("档位N")
            else if (dataBus.gear === 1) parts.push("档位R")
            else if (dataBus.gear === 0) parts.push("档位P")
            parts.push("续航" + dataBus.range.toFixed(0) + "km")
            parts.push("SOC " + dataBus.soc.toFixed(0) + "%")
            parts.push("空调" + dataBus.rightTemp.toFixed(0) + "度")
            parts.push("风扇" + (dataBus.fanActive ? "开" : "关"))
            agentLoop.setAdditionalContext(parts.join("，"))
        }
    }
    property string lastContextSent: ""

    // ── Tool → UI bridges ──
    Connections {
        target: climateTool
        function onClimateControlChanged(fanActive, leftTemp, rightTemp, fanLevel) {
            console.log("ClimateTool → UI:", fanActive, leftTemp, rightTemp, fanLevel)
            bottomControls.fanActive = fanActive
            bottomControls.targetTemp = rightTemp
            bottomControls.targetTempDisplay = rightTemp + "°C"
            bottomControls.fanLevel = fanLevel
            if (bottomControls.interiorTemp === undefined)
                bottomControls.interiorTemp = 25.0
            bottomControls.interiorTemp = rightTemp
            bottomControls.fanStatus = fanActive
                ? (fanLevel === 0 ? "自动 · 1档" : "手动 · " + fanLevel + "档")
                : "已关闭"
        }
    }

    Connections {
        target: navTool
        function onNavigationRequested(startAddress, endAddress, city) {
            console.log("NavigationTool → UI:", startAddress, endAddress, city)
            root.activeSidebarIndex = 1
            mapPanel.openPanelAndNavigate(startAddress, endAddress, city)
        }
    }
}
