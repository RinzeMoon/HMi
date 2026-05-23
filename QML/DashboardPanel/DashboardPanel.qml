import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15
import Qt5Compat.GraphicalEffects
import "qrc:/QML/SpeedGauge"
import "qrc:/QML/ProgressBar"

Item {
    id: root
    FontLoader { id: solidFont; source: "qrc:/QML/fonts/fa-solid-900.ttf" }

    // ========== 属性定义 ==========
    // 中央多媒体卡片 - 本地缓存
    property string musicSongName: ""
    property string musicArtistName: ""
    property string musicCurrentTime: "--:--"
    property string musicTotalTime: "--:--"
    property real musicProgressPercent: 0.0
    property bool musicIsPlaying: false

    // 保存根组件（HMIBackground）的引用
    property var rootRef: null
    
    Component.onCompleted: {
        try {
            // 向上查找根组件（HMIBackground）
            var p = parent
            while (p !== undefined && p !== null) {
                // 检查是否有globalSongName属性，判断是不是HMIBackground
                if (p.hasOwnProperty("globalSongName")) {
                    rootRef = p
                    break
                }
                p = p.parent
            }
            
            // 启动定位服务
            if (typeof locationService !== 'undefined' && locationService !== null) {
                locationService.startLocation()
            }
            
            // 连接定位服务信号
            if (typeof locationService !== 'undefined' && locationService !== null) {
                locationService.positionUpdated.connect(function(latitude, longitude, address) {
                    root.locationEnabled = true
                    // 通过经纬度获取天气
                    if (typeof weatherService !== 'undefined' && weatherService !== null) {
                        weatherService.getWeatherByCoordinates(latitude, longitude)
                    }
                })
                
                locationService.locationError.connect(function(error) {
                    console.log("Location error:", error)
                    // 定位失败时使用默认城市
                    if (typeof weatherService !== 'undefined' && weatherService !== null) {
                        weatherService.getWeatherByCity("北京市")
                    }
                })
            }
            
            // 连接天气服务信号
            if (typeof weatherService !== 'undefined' && weatherService !== null) {
                weatherService.weatherDataReceived.connect(function(location, temperature, condition, icon) {
                    root.weatherLocation = location
                    root.weatherTemp = temperature
                    root.weatherCondition = condition
                    root.weatherIcon = icon
                    root.outsideTemp = temperature // 同步外部温度
                })
                
                weatherService.weatherDataError.connect(function(error) {
                    console.log("Weather error:", error)
                })
                
                // 初始获取北京天气
                weatherService.getWeatherByCity("北京市")
            }
        } catch (e) {
            console.log("[DashboardPanel] Component.onCompleted error:", e)
        }
    }
    
    // 定时器同步全局音乐状态
    Timer {
        id: dashboardSyncTimer
        interval: 50
        repeat: true
        running: true
        onTriggered: {
            try {
                if (rootRef && rootRef !== null) {
                    root.musicSongName = rootRef.globalSongName || ""
                    root.musicArtistName = rootRef.globalArtistName || ""
                    root.musicCurrentTime = rootRef.globalCurrentTime || "--:--"
                    root.musicTotalTime = rootRef.globalTotalTime || "--:--"
                    root.musicProgressPercent = rootRef.globalProgressPercent || 0.0
                    root.musicIsPlaying = rootRef.globalIsPlaying || false
                }
            } catch (e) {
                console.log("[DashboardPanel] dashboardSyncTimer error:", e)
            }
        }
    }
    // 左侧速度卡片
    property real speed: 0           // 车速
    property real rpm: 0              // 转速
    property int maxRpm: 8000                // 最大转速（常量，可保留默认值）
    property real fuelLevel: 75        // 油量百分比
    property real coolantTemp: 85      // 水温（°C）
    property real coolantPercent: 50   // 水温进度条百分比
    property real energyRecovery: 5   // 能量回收 kW
    property real range: 450            // 剩余续航 km

    // 中央多媒体卡片
    property var trackName: undefined        // 歌曲名
    property var trackArtist: undefined      // 艺术家
    property var currentTime: undefined      // 当前时间
    property var totalTime: undefined        // 总时间
    property var progressPercent: undefined  // 播放进度百分比
    property var navPrimary: undefined       // 导航主要文字
    property var navSecondary: undefined     // 导航次要文字
    property var trafficLights: undefined    // 信号灯数量
    property var etaMinutes: undefined       // 预计分钟

    // 右侧车辆状态卡片
    property real remainingRange: 450   // 剩余里程
    property string tirePressureText: "2.4 bar" // 胎压文字
    property real outsideTemp: 22      // 外部温度
    property real tripOdometer: 123.4     // 小计里程
    property string gearPosition: "P"           // 挡位（可保留默认值）
    property var driveMode: "舒适"            // 驾驶模式（可保留默认值）
    
    // 天气信息
    property string weatherCondition: "晴"
    property int weatherTemp: 22
    property string weatherIcon: "\uf185"
    property string weatherLocation: "北京市"
    
    // 定位状态
    property bool locationEnabled: false

    // ========== 数值平滑动画 ==========
    Behavior on speed { NumberAnimation { duration: 800; easing.type: Easing.InOutQuad } }
    Behavior on rpm { NumberAnimation { duration: 800; easing.type: Easing.InOutQuad } }
    Behavior on fuelLevel { NumberAnimation { duration: 1000; easing.type: Easing.InOutQuad } }
    Behavior on coolantTemp { NumberAnimation { duration: 1000; easing.type: Easing.InOutQuad } }
    Behavior on coolantPercent { NumberAnimation { duration: 1000; easing.type: Easing.InOutQuad } }
    Behavior on energyRecovery { NumberAnimation { duration: 1000; easing.type: Easing.InOutQuad } }
    Behavior on range { NumberAnimation { duration: 1000; easing.type: Easing.InOutQuad } }
    Behavior on remainingRange { NumberAnimation { duration: 1000; easing.type: Easing.InOutQuad } }
    Behavior on outsideTemp { NumberAnimation { duration: 1500; easing.type: Easing.InOutQuad } }

    // ========== 模拟数据定时器 ==========
    Timer {
        id: simulationTimer
        interval: 1200
        running: true
        repeat: true
        property real internalRpm: 0
        property real internalSpeed: 0
        property real internalFuel: 75
        property real internalCoolantTemp: 85
        property real internalCoolantPercent: 50
        property real internalEnergyRecovery: 5
        property real internalRange: 450
        property real internalOutsideTemp: 22
        property real internalTripOdometer: 123.4
        

        
        onTriggered: {
            // 模拟转速波动（500-4000 RPM）
            internalRpm = 2000 + Math.sin(Date.now() / 5000) * 1500 + Math.random() * 300
            internalRpm = Math.max(0, Math.min(root.maxRpm, internalRpm))
            root.rpm = Math.round(internalRpm)
            
            // 模拟速度随转速变化（0-100 km/h）
            internalSpeed = Math.max(0, Math.min(100, internalRpm / 40))
            root.speed = Math.round(internalSpeed)
            
            // 模拟油量缓慢减少（最低10%）
            internalFuel = Math.max(10, internalFuel - 0.1)
            root.fuelLevel = Math.round(internalFuel)
            
            // 模拟水温（80-90°C）
            internalCoolantTemp = 85 + Math.sin(Date.now() / 8000) * 5
            internalCoolantPercent = 50 + Math.sin(Date.now() / 8000) * 10
            root.coolantTemp = Math.round(internalCoolantTemp)
            root.coolantPercent = Math.round(internalCoolantPercent)
            
            // 模拟能量回收（0-8 kW）
            internalEnergyRecovery = Math.max(0, 5 + Math.sin(Date.now() / 6000) * 3)
            root.energyRecovery = Math.round(internalEnergyRecovery)
            
            // 模拟剩余续航（随油量变化）
            internalRange = Math.max(50, 450 - (75 - internalFuel) * 5)
            root.range = Math.round(internalRange)
            root.remainingRange = Math.round(internalRange)
            

            
            // 模拟小计里程（随速度增加）
            internalTripOdometer = internalTripOdometer + internalSpeed / 3000
            root.tripOdometer = Math.round(internalTripOdometer)
        }
    }

    // 辅助函数：安全显示数值
    function displayNumber(val, unit = "", placeholder = "--") {
        if (val === undefined || isNaN(val)) return placeholder;
        return val.toFixed(0) + unit;
    }
    function displayString(val, placeholder = "--") {
        return (val !== undefined && val !== "") ? val : placeholder;
    }
    function displayPercent(val, max = 100, placeholder = 0) {
        if (val === undefined || isNaN(val)) return placeholder;
        return val / max;
    }

    GridLayout {
        anchors.fill: parent
        columns: 3
        rowSpacing: 20
        columnSpacing: 20

        // ========== 左侧速度表卡片 ==========
        Rectangle {
            id: cardLeft
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: "white"
            radius: 36
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
            HoverHandler { onHoveredChanged: cardLeft.hovered = hovered }

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 22
                spacing: 20

                SpeedGauge {
                    Layout.alignment: Qt.AlignHCenter
                    speed: root.speed !== undefined ? root.speed : 0
                    unitText: "km/h"
                }
                ProgressBar {
                    Layout.fillWidth: true
                    label: "转速"
                    value: root.rpm !== undefined ? root.rpm : 0
                    max: root.maxRpm
                    barColor: "#2670e8"
                }

                RowLayout {
                    spacing: 16
                    ColumnLayout {
                        RowLayout {
                            spacing: 4
                            Text { text: "\uf52f"; font.family: solidFont.name; font.pixelSize: 13; color: "#2670e8" }
                            Text { text: displayNumber(root.fuelLevel, "%"); font.pixelSize: 13; color: "#1f3147" }
                        }
                        Rectangle {
                            width: 60; height: 5; color: "#dae2ec"; radius: 3
                            Rectangle {
                                width: parent.width * displayPercent(root.fuelLevel)
                                height: parent.height
                                color: "#27a068"; radius: 3
                            }
                        }
                    }
                    ColumnLayout {
                        RowLayout {
                            spacing: 4
                            Text { text: "\uf769"; font.family: solidFont.name; font.pixelSize: 13; color: "#2670e8" }
                            Text { text: displayNumber(root.coolantTemp, "°C"); font.pixelSize: 13; color: "#1f3147" }
                        }
                        Rectangle {
                            width: 60; height: 5; color: "#dae2ec"; radius: 3
                            Rectangle {
                                width: parent.width * displayPercent(root.coolantPercent)
                                height: parent.height
                                color: "#3880ff"; radius: 3
                            }
                        }
                    }
                }

                RowLayout {
                    spacing: 20
                    Text {
                        text: "\uf0e7 " + displayNumber(root.energyRecovery, "kW回收")
                        font.family: solidFont.name
                        font.pixelSize: 13
                        color: "#3b5d85"
                    }
                    Text {
                        text: "\uf242 " + displayNumber(root.range, "km")
                        font.family: solidFont.name
                        font.pixelSize: 13
                        color: "#3b5d85"
                    }
                }
            }
        }

        // ========== 中央多媒体卡片 ==========
        Rectangle {
            id: cardCenter
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: "white"
            radius: 36
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
            HoverHandler { onHoveredChanged: cardCenter.hovered = hovered }

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 22
                spacing: 4

                // 标题
                RowLayout {
                    Text { text: "\uf2f1"; font.family: solidFont.name; font.pixelSize: 20; color: "#2670e8" }
                    Text { text: "多媒体"; font.pixelSize: 18; font.weight: Font.Bold; color: "#0c1c32" }
                }

                // 播放信息行
                RowLayout {
                    spacing: 16
                    Rectangle {
                        width: 60; height: 60; radius: 22
                        gradient: Gradient {
                            GradientStop { position: 0.0; color: "#e0ebf9" }
                            GradientStop { position: 1.0; color: "#cddef5" }
                        }
                        Text { 
                            anchors.centerIn: parent
                            text: root.musicIsPlaying ? "\uf04c" : "\uf04b"
                            font.family: solidFont.name
                            font.pixelSize: 28
                            color: "#1f3a6b"
                        }
                    }
                    ColumnLayout {
                        spacing: 2
                        Text { 
                            text: displayString(root.musicSongName)
                            font.pixelSize: 18
                            font.weight: Font.Bold
                            color: "#0e2038"
                        }
                        Text { 
                            text: displayString(root.musicArtistName)
                            font.pixelSize: 14
                            color: "#587394"
                        }
                    }
                }

                // 进度条
                RowLayout {
                    spacing: 4
                    Text { 
                        text: displayString(root.musicCurrentTime, "0:00")
                        font.pixelSize: 13
                        color: "#2f4a6b"
                    }
                    Rectangle {
                        Layout.fillWidth: true; height: 5; color: "#dde5ef"; radius: 3
                        Rectangle {
                            width: parent.width * (root.musicProgressPercent !== undefined ? root.musicProgressPercent : 0)
                            height: parent.height
                            color: "#4791ff"; radius: 3
                        }
                    }
                    Text { 
                        text: displayString(root.musicTotalTime, "0:00")
                        font.pixelSize: 13
                        color: "#2f4a6b"
                    }
                }

                // 导航预览卡片
                Rectangle {
                    color: "#f6faff"; radius: 24; border.color: "#dde9fc"; border.width: 1
                    height: 70; Layout.fillWidth: true
                    RowLayout {
                        anchors.fill: parent; anchors.margins: 14; spacing: 12
                        Text { text: "\uf5a0"; font.family: solidFont.name; font.pixelSize: 24; color: "#2670e8" }
                        ColumnLayout {
                            spacing: 2
                            Text { text: displayString(root.navPrimary); font.pixelSize: 15; font.weight: Font.Bold; color: "#0b1d33" }
                            Text { text: displayString(root.navSecondary); font.pixelSize: 13; color: "#5b728f" }
                        }
                        Item { Layout.fillWidth: true }
                        Text { text: "\uf061"; font.family: solidFont.name; font.pixelSize: 22; color: "#2670e8" }
                    }
                }

                // 路况信息
                RowLayout {
                    spacing: 12
                    Text {
                        text: "\uf637 " + displayNumber(root.trafficLights, "个信号灯")
                        font.family: solidFont.name
                        font.pixelSize: 13
                        color: "#3d608b"
                    }
                    Text {
                        text: "\uf017 " + displayNumber(root.etaMinutes, "分钟")
                        font.family: solidFont.name
                        font.pixelSize: 13
                        color: "#3d608b"
                    }
                }
            }
        }

        // ========== 右侧车辆状态卡片 ==========
        Rectangle {
            id: cardRight
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: "white"
            radius: 36
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
            HoverHandler { onHoveredChanged: cardRight.hovered = hovered }

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 22
                spacing: 16

                // 天气信息模块
                Rectangle {
                    color: "#f0f6ff"; radius: 24; border.color: "#d3e2f7"; border.width: 1
                    Layout.fillWidth: true; height: 80
                    RowLayout {
                        anchors.fill: parent; anchors.margins: 16; spacing: 16
                        Text {
                            text: root.weatherIcon
                            font.family: solidFont.name
                            font.pixelSize: 36
                            color: "#2670e8"
                        }
                        ColumnLayout {
                            spacing: 4
                            Text {
                                text: root.weatherLocation
                                font.pixelSize: 14
                                color: "#5b728f"
                            }
                            RowLayout {
                                spacing: 8
                                Text {
                                    text: displayNumber(root.weatherTemp, "°C")
                                    font.pixelSize: 24
                                    font.weight: Font.Bold
                                    color: "#0b1a2e"
                                }
                                Text {
                                    text: root.weatherCondition
                                    font.pixelSize: 16
                                    color: "#1f3853"
                                }
                            }
                        }
                    }
                }

                RowLayout {
                    Text { text: "\uf1b9"; font.family: solidFont.name; font.pixelSize: 20; color: "#2670e8" }
                    Text { text: "车辆状态"; font.pixelSize: 18; font.weight: Font.Bold; color: "#0c1c32" }
                }

                Rectangle {
                    color: "#f2f7ff"; radius: 28; border.color: "#d3e2f7"; border.width: 1
                    Layout.fillWidth: true; height: 100
                    ColumnLayout {
                        anchors.fill: parent; anchors.margins: 16; spacing: 12
                        Label {
                            text: root.gearPosition
                            font.pixelSize: 42; font.weight: Font.Bold; color: "#0b1a2e"
                            background: Rectangle { color: "white"; radius: 50; border.color: "#c1d6f0"; border.width: 1; implicitWidth: 80; implicitHeight: 60 }
                            horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter; Layout.alignment: Qt.AlignHCenter
                        }
                        RowLayout {
                            spacing: 12
                            Repeater {
                                model: ["P", "R", "N", "D"]
                                Rectangle {
                                    width: 40; height: 36; radius: 30
                                    color: (modelData === root.gearPosition) ? "#2670e8" : "white"
                                    border.color: (modelData === root.gearPosition) ? "#2670e8" : "#cddef2"
                                    border.width: 1
                                    Text {
                                        anchors.centerIn: parent
                                        text: modelData
                                        font.pixelSize: 18
                                        font.weight: Font.Bold
                                        color: (modelData === root.gearPosition) ? "white" : "#34537a"
                                    }
                                }
                            }
                        }
                    }
                }

                ColumnLayout {
                    spacing: 10
                    // 剩余里程
                    RowLayout {
                        Layout.fillWidth: true
                        Text { text: "\uf625"; font.family: solidFont.name; font.pixelSize: 16; color: "#2670e8"; width: 24 }
                        Text { text: "剩余里程"; font.pixelSize: 15; color: "#1f3853"; Layout.fillWidth: true }
                        Text { text: displayNumber(root.remainingRange, " km"); font.pixelSize: 15; font.weight: Font.Bold; color: "#0b1d30" }
                    }
                    // 胎压
                    RowLayout {
                        Layout.fillWidth: true
                        Text { text: "  "; width: 24 }
                        Text { text: "胎压"; font.pixelSize: 15; color: "#1f3853"; Layout.fillWidth: true }
                        Text { text: displayString(root.tirePressureText); font.pixelSize: 15; font.weight: Font.Bold; color: "#0b1d30" }
                    }
                    // 外部温度
                    RowLayout {
                        Layout.fillWidth: true
                        Text { text: "\uf769"; font.family: solidFont.name; font.pixelSize: 16; color: "#2670e8"; width: 24 }
                        Text { text: "外部温度"; font.pixelSize: 15; color: "#1f3853"; Layout.fillWidth: true }
                        Text { text: displayNumber(root.outsideTemp, "°C"); font.pixelSize: 15; font.weight: Font.Bold; color: "#0b1d30" }
                    }
                    // 小计里程
                    RowLayout {
                        Layout.fillWidth: true
                        Text { text: "  "; width: 24 }
                        Text { text: "小计里程"; font.pixelSize: 15; color: "#1f3853"; Layout.fillWidth: true }
                        Text { text: displayNumber(root.tripOdometer, " km"); font.pixelSize: 15; font.weight: Font.Bold; color: "#0b1d30" }
                    }
                }

                RowLayout {
                    spacing: 10
                    Repeater {
                        model: ["舒适", "运动", "节能"]
                        Rectangle {
                            Layout.fillWidth: true; height: 36; radius: 30
                            color: (modelData === root.driveMode) ? "#2670e8" : "#f0f6ff"
                            border.color: (modelData === root.driveMode) ? "#2670e8" : "#cfe0f5"
                            border.width: 1
                            Text {
                                anchors.centerIn: parent
                                text: modelData
                                font.pixelSize: 14
                                font.weight: Font.Bold
                                color: (modelData === root.driveMode) ? "white" : "#1b3a5a"
                            }
                        }
                    }
                }
            }
        }
    }
}