import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15
import Qt5Compat.GraphicalEffects
import QtQml 2.15
import QtMultimedia

Item {
    FontLoader { id: solidFont; source: "qrc:/QML/fonts/fa-solid-900.ttf" }
    
    signal songPlayed(string songName, string artistName)

    // ========== 歌曲信息（初始为空，由外部填充） ==========
    property string coverIcon: "\uf001"
    property color coverColor1: "#e0ebf9"
    property color coverColor2: "#cddef5"
    property color coverIconColor: "#1f3a6b"

    property string songName: ""
    property string artistName: ""
    property string albumName: ""
    property color songNameColor: "#0e2038"
    property color artistColor: "#587394"
    property color albumColor: "#2f405a"

    // 进度条（默认占位）
    property string currentTime: "--:--"
    property string totalTime: "--:--"
    property real progressPercent: 0.0
    property color progressColor: "#4791ff"
    property color progressBgColor: "#dde5ef"

    // 控制按钮颜色（可外部定制）
    property color prevBtnColor: "#f0f6ff"
    property color playBtnColor: "#2670e8"
    property color nextBtnColor: "#f0f6ff"
    property color btnIconColor: "#2670e8"
    property color playBtnIconColor: "white"

    // 音量控制
    property int volumePercent: 70
    property color volumeIconColor: "#2670e8"
    property color volumeBarColor: "#4791ff"
    property color volumeBgColor: "#dde5ef"
    property string volumeTextColor: "#2f4a6b"

    // 底部图标状态（移除了红心 likeActive）
    property bool repeatActive: false
    property bool shuffleActive: false
    property color bottomIconColor: "#2670e8"

    // 歌词数据（初始为空）
    property var lyricsModel: ListModel {}
    property int currentLyricIndex: 0
    property color currentLyricColor: "#0e2038"
    property color otherLyricColor: "#2d3a4f"
    property int currentLyricFontSize: 22
    property int otherLyricFontSize: 18
    property int lyricSpacing: 20
    property int lyricIndicatorWidth: 4
    property color lyricIndicatorColor: "#2670e8"

    // 歌单数据（初始为空，每个元素应包含 name, artist, duration, id 等字段）
    property var playlistModel: ListModel {}

    // 流体背景
    property bool enableFlowAnimation: true
    property color bgColor1: "#e0ebf9"
    property color bgColor2: "#b0c8f0"
    property color bgColor3: "#8aaae0"
    property int animationDuration: 4000

    // 右侧显示模式：true=歌词，false=歌单
    property bool showLyrics: true

    // 搜索文本
    property string searchText: ""
    // 是否由AI触发的搜索
    property bool isAiTriggeredSearch: false

    // 播放状态（用于同步按钮图标）
    property bool isPlaying: false

    // seek状态（用于拖动进度条）
    property bool isSeeking: false

    // ---------- Qt6 多媒体播放器 ----------
    MediaPlayer {
        id: player
        audioOutput: AudioOutput {}   // 必须设置音频输出

        onPositionChanged: {
            // 更新进度条和时间
            currentTime = formatTime(position)
            if (duration > 0)
                progressPercent = position / duration
            
            // 同步歌词
            var currentIndex = findCurrentLyricIndex(position)
            if (currentIndex !== -1 && currentIndex !== currentLyricIndex) {
                currentLyricIndex = currentIndex
            }
        }
        onDurationChanged: {
            totalTime = formatTime(duration)
        }
        onPlayingChanged: {
            console.log("播放状态改变: playing=" + playing)
            isPlaying = playing
        }
        onErrorOccurred: {
            console.log("播放错误:", errorString)
        }
    }

    // 格式化时间（毫秒 -> MM:SS）
    function formatTime(ms) {
        var totalSeconds = Math.floor(ms / 1000)
        var minutes = Math.floor(totalSeconds / 60)
        var seconds = totalSeconds % 60
        return minutes + ":" + (seconds < 10 ? "0" + seconds : seconds)
    }

    // 格式化歌曲时长（毫秒 -> MM:SS）
    function formatDuration(ms) {
        if (!ms || ms <= 0) return "--:--"
        var totalSeconds = Math.floor(ms / 1000)
        var minutes = Math.floor(totalSeconds / 60)
        var seconds = totalSeconds % 60
        return minutes + ":" + (seconds < 10 ? "0" + seconds : seconds)
    }

    // 解析歌词
    function parseLyrics(lyricText) {
        lyricsModel.clear()
        if (!lyricText || lyricText.trim() === "") {
            console.log("歌词为空")
            return
        }
        
        var lines = lyricText.split('\n')
        var lyricRegex = /\[(\d{2}):(\d{2})\.(\d{2,3})\](.*)/
        
        for (var i = 0; i < lines.length; i++) {
            var line = lines[i].trim()
            if (line === "") continue
            
            var match = line.match(lyricRegex)
            if (match) {
                var minutes = parseInt(match[1])
                var seconds = parseInt(match[2])
                var milliseconds = parseInt(match[3])
                // 处理毫秒（可能是2位或3位）
                if (match[3].length === 2) {
                    milliseconds *= 10
                }
                var timeMs = (minutes * 60 + seconds) * 1000 + milliseconds
                
                lyricsModel.append({
                    "time": timeMs,
                    "text": match[4].trim()
                })
            }
        }
        
        console.log("解析歌词完成，共 " + lyricsModel.count + " 行")
    }

    // 查找当前时间对应的歌词行
    function findCurrentLyricIndex(positionMs) {
        if (lyricsModel.count === 0) return -1
        
        for (var i = 0; i < lyricsModel.count; i++) {
            var lyricTime = lyricsModel.get(i).time
            if (positionMs < lyricTime) {
                return i > 0 ? i - 1 : 0
            }
        }
        return lyricsModel.count - 1
    }

    Rectangle {
        anchors.fill: parent
        color: "white"
        radius: 36
        border.color: "#d0ddee"
        border.width: 1
        layer.enabled: true
        layer.effect: DropShadow {
            transparentBorder: true
            color: "#10000000"
            radius: 12
            samples: 25
            horizontalOffset: 0
            verticalOffset: 3
        }

        RowLayout {
            anchors.fill: parent
            anchors.margins: 22
            spacing: 20

            // ========== 左侧：封面 + 信息 + 控制 ==========
            ColumnLayout {
                Layout.preferredWidth: 0.4 * parent.width
                Layout.fillHeight: true
                spacing: 16

                // 搜索栏（位于左侧面板内）
                Rectangle {
                    Layout.fillWidth: true
                    height: 50
                    color: "#f6faff"
                    radius: 25
                    border.color: "#d2e3fc"
                    border.width: 1

                    RowLayout {
                        anchors.fill: parent
                        anchors.leftMargin: 16
                        anchors.rightMargin: 8
                        spacing: 8

                        // 输入框（无左侧图标）
                        TextField {
                            id: searchField
                            Layout.fillWidth: true
                            placeholderText: "搜索歌曲..."
                            text: searchText
                            onTextChanged: searchText = text
                            background: null
                            font.pixelSize: 16
                            color: "#1e2f47"
                            selectByMouse: true
                            // 回车搜索
                            onAccepted: {
                                if (text.trim() !== "" && musicService) {
                                    musicService.searchSongs(text.trim())
                                    showLyrics = false
                                }
                            }
                        }

                        // 右侧搜索按钮（带背景和悬停效果）
                        Rectangle {
                            id: searchBtn
                            width: 40
                            height: 40
                            radius: 20
                            color: "#2670e8"
                            property bool hovered: false
                            Behavior on color { ColorAnimation { duration: 150 } }

                            Text {
                                anchors.centerIn: parent
                                text: ""
                                font.family: solidFont.name
                                font.pixelSize: 18
                                color: "white"
                            }

                            MouseArea {
                                anchors.fill: parent
                                hoverEnabled: true
                                onEntered: searchBtn.color = "#1a5ac9"
                                onExited: searchBtn.color = "#2670e8"
                                onClicked: {
                                    if (searchField.text.trim() !== "" && musicService) {
                                        musicService.searchSongs(searchField.text.trim())
                                        showLyrics = false
                                    }
                                }
                            }
                        }
                    }
                }

                // 封面图
                Rectangle {
                    Layout.alignment: Qt.AlignHCenter
                    width: 160
                    height: 160
                    radius: 24
                    gradient: Gradient {
                        GradientStop { position: 0.0; color: coverColor1 }
                        GradientStop { position: 1.0; color: coverColor2 }
                    }
                    Image {
                        id: coverImage
                        anchors.fill: parent
                        source: ""
                        fillMode: Image.PreserveAspectCrop
                        visible: true
                        cache: true
                    }
                    Text {
                        anchors.centerIn: parent
                        text: coverIcon
                        font.family: solidFont.name
                        font.pixelSize: 64
                        color: coverIconColor
                        visible: coverImage.status === Image.Error || (coverImage.status === Image.Null && coverIcon !== "")
                    }
                }

                // 歌曲信息（无数据时显示占位符）
                ColumnLayout {
                    spacing: 4
                    Layout.fillWidth: true

                    Text {
                        text: songName !== "" ? songName : "--"
                        font.pixelSize: 22
                        font.weight: Font.Bold
                        color: songNameColor
                        elide: Text.ElideRight
                    }
                    Text {
                        text: artistName !== "" ? artistName : "--"
                        font.pixelSize: 15
                        color: artistColor
                    }
                    RowLayout {
                        spacing: 6
                        Text {
                            text: "\uf1bc"
                            font.family: solidFont.name
                            font.pixelSize: 13
                            color: "#2670e8"
                        }
                        Text {
                            text: "专辑: " + (albumName !== "" ? albumName : "--")
                            font.pixelSize: 13
                            color: albumColor
                        }
                    }
                }

                // 进度条（点击可拖动）
                RowLayout {
                    spacing: 6
                    Layout.fillWidth: true
                    Text {
                        text: currentTime
                        font.pixelSize: 12
                        color: volumeTextColor
                    }
                    Rectangle {
                        Layout.fillWidth: true
                        height: 4
                        color: progressBgColor
                        radius: 2
                        Rectangle {
                            width: parent.width * progressPercent
                            height: parent.height
                            color: progressColor
                            radius: 2
                        }
                        // 小圆钮
                        Rectangle {
                            width: isSeeking ? 16 : 12
                            height: isSeeking ? 16 : 12
                            radius: 8
                            color: "#2670e8"
                            anchors.verticalCenter: parent.verticalCenter
                            x: parent.width * progressPercent - width/2
                            visible: true
                            Behavior on x { 
                                NumberAnimation { 
                                    duration: 150 
                                    easing.type: Easing.OutCubic 
                                } 
                            }
                            Behavior on width { 
                                NumberAnimation { 
                                    duration: 150 
                                    easing.type: Easing.OutCubic 
                                } 
                            }
                            Behavior on height { 
                                NumberAnimation { 
                                    duration: 150 
                                    easing.type: Easing.OutCubic 
                                } 
                            }
                            // 内部小圆点
                            Rectangle {
                                width: isSeeking ? 6 : 4
                                height: isSeeking ? 6 : 4
                                radius: 3
                                color: "white"
                                anchors.centerIn: parent
                            }
                        }
                        MouseArea {
                            anchors.fill: parent
                            hoverEnabled: true
                            onPressed: {
                                isSeeking = true
                            }
                            onReleased: {
                                if (player.duration > 0) {
                                    var newPos = Math.max(0, Math.min(player.duration, mouse.x / width * player.duration))
                                    player.position = newPos
                                }
                                isSeeking = false
                            }
                            onPositionChanged: {
                                if (isSeeking && player.duration > 0) {
                                    var newPos = Math.max(0, Math.min(player.duration, mouse.x / width * player.duration))
                                    player.position = newPos
                                }
                            }
                        }
                    }
                    Text {
                        text: totalTime
                        font.pixelSize: 12
                        color: volumeTextColor
                    }
                }

                // 播放控制按钮
                RowLayout {
                    spacing: 16
                    Layout.alignment: Qt.AlignHCenter

                    // 上一曲
                    Rectangle {
                        width: 36; height: 36; radius: 18
                        color: prevBtnColor
                        border.color: "#cfe0f5"; border.width: 1
                        Text {
                            anchors.centerIn: parent
                            text: "\uf04a"
                            font.family: solidFont.name
                            font.pixelSize: 18
                            color: btnIconColor
                        }
                        MouseArea {
                            anchors.fill: parent
                            hoverEnabled: true
                            onEntered: parent.color = "#e0edff"
                            onExited: parent.color = prevBtnColor
                            onClicked: playPrevious()
                        }
                    }

                    // 播放/暂停
                    Rectangle {
                        width: 48; height: 48; radius: 24
                        color: playBtnColor
                        border.color: playBtnColor; border.width: 1
                        Text {
                            anchors.centerIn: parent
                            text: isPlaying ? "\uf04c" : "\uf04b"
                            font.family: solidFont.name
                            font.pixelSize: 24
                            color: playBtnIconColor
                        }
                        MouseArea {
                            anchors.fill: parent
                            hoverEnabled: true
                            onEntered: parent.color = "#1a5ac9"
                            onExited: parent.color = playBtnColor
                            onClicked: {
                                console.log("播放状态切换: isPlaying=" + isPlaying + " -> " + (!isPlaying))
                                if (isPlaying) {
                                    player.pause()
                                } else {
                                    player.play()
                                }
                            }
                        }
                    }

                    // 下一曲
                    Rectangle {
                        width: 36; height: 36; radius: 18
                        color: nextBtnColor
                        border.color: "#cfe0f5"; border.width: 1
                        Text {
                            anchors.centerIn: parent
                            text: "\uf04e"
                            font.family: solidFont.name
                            font.pixelSize: 18
                            color: btnIconColor
                        }
                        MouseArea {
                            anchors.fill: parent
                            hoverEnabled: true
                            onEntered: parent.color = "#e0edff"
                            onExited: parent.color = nextBtnColor
                            onClicked: playNext()
                        }
                    }
                }

                // 音量控制
                RowLayout {
                    spacing: 8
                    Layout.fillWidth: true
                    Text {
                        text: "\uf028"
                        font.family: solidFont.name
                        font.pixelSize: 16
                        color: volumeIconColor
                    }
                    Rectangle {
                        Layout.fillWidth: true
                        height: 4
                        color: volumeBgColor
                        radius: 2
                        Rectangle {
                            width: parent.width * (player.audioOutput ? player.audioOutput.volume : 0)
                            height: parent.height
                            color: volumeBarColor
                            radius: 2
                        }
                        // 小圆钮
                        Rectangle {
                            width: 12
                            height: 12
                            radius: 6
                            color: "#2670e8"
                            anchors.verticalCenter: parent.verticalCenter
                            x: parent.width * (player.audioOutput ? player.audioOutput.volume : 0) - width/2
                            visible: parent.containsMouse || (player.audioOutput && player.audioOutput.volume > 0)
                            Behavior on x { 
                                NumberAnimation { 
                                    duration: 150 
                                    easing.type: Easing.OutCubic 
                                } 
                            }
                        }
                        MouseArea {
                            anchors.fill: parent
                            hoverEnabled: true
                            onPositionChanged: {
                                if (player.audioOutput) {
                                    var newVolume = Math.max(0, Math.min(1, mouse.x / width))
                                    player.audioOutput.volume = newVolume
                                    volumePercent = Math.round(newVolume * 100)
                                }
                            }
                            onClicked: {
                                if (player.audioOutput) {
                                    var newVolume = Math.max(0, Math.min(1, mouse.x / width))
                                    player.audioOutput.volume = newVolume
                                    volumePercent = Math.round(newVolume * 100)
                                }
                            }
                        }
                    }
                    Text {
                        text: player.audioOutput ? Math.round(player.audioOutput.volume * 100) + "%" : "0%"
                        font.pixelSize: 12
                        color: volumeTextColor
                    }
                }

                // 底部小图标（移除了红心）
                RowLayout {
                    spacing: 16
                    Layout.alignment: Qt.AlignHCenter
                    // 循环图标
                    Text {
                        text: repeatActive ? "\uf01e" : "\uf01e"
                        font.family: solidFont.name
                        font.pixelSize: 16
                        color: bottomIconColor
                        opacity: repeatActive ? 1.0 : 0.5
                        MouseArea {
                            anchors.fill: parent
                            onClicked: repeatActive = !repeatActive
                        }
                    }
                    // 随机图标
                    Text {
                        text: shuffleActive ? "\uf074" : "\uf074"
                        font.family: solidFont.name
                        font.pixelSize: 16
                        color: bottomIconColor
                        opacity: shuffleActive ? 1.0 : 0.5
                        MouseArea {
                            anchors.fill: parent
                            onClicked: shuffleActive = !shuffleActive
                        }
                    }
                }
            }

            // ========== 右侧：歌词/歌单切换区域 ==========
            Item {
                Layout.preferredWidth: 0.6 * parent.width
                Layout.fillHeight: true
                clip: true

                // 流体渐变背景
                Rectangle {
                    anchors.fill: parent
                    radius: 36
                    gradient: Gradient {
                        GradientStop { id: stop1; position: 0.0; color: bgColor1 }
                        GradientStop { id: stop2; position: 0.5; color: bgColor2 }
                        GradientStop { id: stop3; position: 1.0; color: bgColor3 }
                    }
                    SequentialAnimation {
                        id: flowAnimation
                        loops: Animation.Infinite
                        running: enableFlowAnimation
                        NumberAnimation {
                            target: stop2
                            property: "position"
                            from: 0.3
                            to: 0.7
                            duration: animationDuration
                            easing.type: Easing.InOutQuad
                        }
                        NumberAnimation {
                            target: stop2
                            property: "position"
                            from: 0.7
                            to: 0.3
                            duration: animationDuration
                            easing.type: Easing.InOutQuad
                        }
                    }
                }

                // 半透明遮罩层
                Rectangle {
                    anchors.fill: parent
                    color: "white"
                    opacity: 0.15
                }

                // 顶部切换按钮
                RowLayout {
                    anchors.top: parent.top
                    anchors.right: parent.right
                    anchors.margins: 16
                    spacing: 8
                    z: 10

                    Rectangle {
                        width: 60; height: 30
                        radius: 15
                        color: showLyrics ? "#2670e8" : "#eef2f8"
                        border.color: showLyrics ? "#2670e8" : "#cddef2"
                        border.width: 1
                        Text {
                            anchors.centerIn: parent
                            text: "歌词"
                            font.pixelSize: 13
                            font.weight: Font.Bold
                            color: showLyrics ? "white" : "#2d3a4f"
                        }
                        MouseArea {
                            anchors.fill: parent
                            onClicked: showLyrics = true
                        }
                    }

                    Rectangle {
                        width: 60; height: 30
                        radius: 15
                        color: !showLyrics ? "#2670e8" : "#eef2f8"
                        border.color: !showLyrics ? "#2670e8" : "#cddef2"
                        border.width: 1
                        Text {
                            anchors.centerIn: parent
                            text: "歌单"
                            font.pixelSize: 13
                            font.weight: Font.Bold
                            color: !showLyrics ? "white" : "#2d3a4f"
                        }
                        MouseArea {
                            anchors.fill: parent
                            onClicked: showLyrics = false
                        }
                    }
                }

                // 歌词视图
                ListView {
                    id: lyricsView
                    anchors.fill: parent
                    anchors.topMargin: 50
                    anchors.bottomMargin: 16
                    anchors.leftMargin: 16
                    anchors.rightMargin: 16
                    spacing: lyricSpacing
                    clip: true
                    visible: showLyrics
                    model: lyricsModel
                    currentIndex: currentLyricIndex

                    Component.onCompleted: {
                        Qt.callLater(function() {
                            positionViewAtIndex(currentIndex, ListView.Center)
                        })
                    }
                    onCurrentIndexChanged: {
                        positionViewAtIndex(currentIndex, ListView.Center)
                    }

                    delegate: Item {
                        width: lyricsView.width
                        height: (index === lyricsView.currentIndex) ? currentLyricFontSize + 30 : otherLyricFontSize + 26

                        RowLayout {
                            anchors.fill: parent
                            spacing: 16

                            Rectangle {
                                width: lyricIndicatorWidth
                                height: parent.height
                                color: (index === lyricsView.currentIndex) ? lyricIndicatorColor : "transparent"
                                radius: 2
                                Behavior on color { 
                                    ColorAnimation { 
                                        duration: 200 
                                    } 
                                }
                            }

                            Text {
                                text: model.text
                                font.pixelSize: (index === lyricsView.currentIndex) ? currentLyricFontSize : otherLyricFontSize
                                font.weight: (index === lyricsView.currentIndex) ? Font.Bold : Font.Normal
                                color: (index === lyricsView.currentIndex) ? currentLyricColor : otherLyricColor
                                elide: Text.ElideRight
                                Layout.fillWidth: true
                                verticalAlignment: Text.AlignVCenter
                                height: parent.height
                                Behavior on font.pixelSize { 
                                    NumberAnimation { 
                                        duration: 200 
                                        easing.type: Easing.OutCubic 
                                    } 
                                }
                                Behavior on font.weight { 
                                    NumberAnimation { 
                                        duration: 200 
                                        easing.type: Easing.OutCubic 
                                    } 
                                }
                                Behavior on color { 
                                    ColorAnimation { 
                                        duration: 200 
                                    } 
                                }
                            }
                        }
                    }

                    // 无歌词时占位提示
                    Label {
                        anchors.centerIn: parent
                        text: "暂无歌词"
                        font.pixelSize: 16
                        color: "#999"
                        visible: lyricsView.count === 0 && showLyrics
                    }
                }

                // 歌单视图（带标题的表格形式）
                ColumnLayout {
                    anchors.fill: parent
                    anchors.topMargin: 50
                    anchors.bottomMargin: 16
                    anchors.leftMargin: 16
                    anchors.rightMargin: 16
                    spacing: 0
                    visible: !showLyrics

                    // 标题行
                    Rectangle {
                        Layout.fillWidth: true
                        height: 30
                        color: "#f0f4fa"
                        radius: 8
                        RowLayout {
                            anchors.fill: parent
                            anchors.leftMargin: 10
                            anchors.rightMargin: 10
                            Text {
                                text: "歌曲名"
                                font.pixelSize: 14
                                font.weight: Font.Bold
                                color: "#333"
                                Layout.fillWidth: true
                            }
                            Text {
                                text: "艺术家"
                                font.pixelSize: 14
                                font.weight: Font.Bold
                                color: "#333"
                                Layout.preferredWidth: 100
                            }
                            Text {
                                text: "时长"
                                font.pixelSize: 14
                                font.weight: Font.Bold
                                color: "#333"
                                Layout.preferredWidth: 60
                                horizontalAlignment: Text.AlignRight
                            }
                        }
                    }

                    // 歌曲列表
                    ListView {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        clip: true
                        model: playlistModel
                        delegate: Rectangle {
                            width: parent.width
                            height: 40
                            color: index % 2 ? "#f8f8f8" : "white"
                            RowLayout {
                                anchors.fill: parent
                                anchors.leftMargin: 10
                                anchors.rightMargin: 10
                                spacing: 10
                                Text {
                                    text: model.name ? model.name : "--"
                                    font.pixelSize: 14
                                    color: "#333"
                                    Layout.fillWidth: true
                                    elide: Text.ElideRight
                                }
                                Text {
                                    text: model.artist ? model.artist : "--"
                                    font.pixelSize: 14
                                    color: "#666"
                                    Layout.preferredWidth: 100
                                    elide: Text.ElideRight
                                }
                                Text {
                                    text: model.duration ? formatDuration(model.duration) : "--:--"
                                    font.pixelSize: 14
                                    color: "#666"
                                    Layout.preferredWidth: 60
                                    horizontalAlignment: Text.AlignRight
                                }
                                Rectangle {
                                    id: playButton
                                    width: 32
                                    height: 32
                                    radius: 16
                                    color: "#2670e8"
                                    opacity: 0.0
                                    property bool hovered: false
                                    Behavior on opacity { 
                                        NumberAnimation { 
                                            duration: 150 
                                            easing.type: Easing.OutCubic 
                                        } 
                                    }
                                    Text {
                                        anchors.centerIn: parent
                                        text: "\uf04b"
                                        font.family: solidFont.name
                                        font.pixelSize: 14
                                        color: "white"
                                    }
                                    MouseArea {
                                        anchors.fill: parent
                                        hoverEnabled: true
                                        onEntered: playButton.opacity = 1.0
                                        onExited: playButton.opacity = 0.0
                                        onClicked: {
                                            console.log("播放歌曲 - id:", model.id)
                                            if (musicService) {
                                                musicService.getSongDetail(model.id)
                                            }
                                        }
                                    }
                                }
                            }
                            MouseArea {
                                id: rowArea
                                anchors.fill: parent
                                hoverEnabled: true
                                onEntered: console.log("hovered")
                                onClicked: {
                                    console.log("选择歌曲 - id:", model.id, "name:", model.name, "artist:", model.artist)
                                    if (musicService) {
                                        musicService.getSongDetail(model.id)
                                    }
                                }
                            }
                        }

                        // 无数据时占位
                        Label {
                            anchors.centerIn: parent
                            text: "暂无歌单"
                            font.pixelSize: 16
                            color: "#999"
                            visible: parent.count === 0
                        }
                    }
                }
            }
        }
    }

    // ========== 辅助函数：上一曲/下一曲 ==========
    property int currentPlaylistIndex: -1

    function playPrevious() {
        if (playlistModel.count === 0) return
        var newIndex = currentPlaylistIndex - 1
        if (newIndex < 0) newIndex = playlistModel.count - 1
        currentPlaylistIndex = newIndex
        var song = playlistModel.get(currentPlaylistIndex)
        if (musicService) {
            musicService.getSongDetail(song.id)
        }
    }

    function playNext() {
        if (playlistModel.count === 0) return
        var newIndex = currentPlaylistIndex + 1
        if (newIndex >= playlistModel.count) newIndex = 0
        currentPlaylistIndex = newIndex
        var song = playlistModel.get(currentPlaylistIndex)
        if (musicService) {
            musicService.getSongDetail(song.id)
        }
    }

    // 填充左侧 UI
    function fillSongInfo(songDetail) {
        if (!songDetail || songDetail.id === undefined) return
        
        var duration = songDetail.duration || 0
        var formattedDuration = formatDuration(duration)
        
        songName = songDetail.name || "--"
        artistName = songDetail.artist || "--"
        albumName = songDetail.album || "--"
        
        // 如果有封面，更新封面图片；否则使用默认图标
        if (songDetail.albumPic && songDetail.albumPic.length > 0) {
            coverIcon = ""
            coverImage.source = songDetail.albumPic
            console.log("设置封面图片:", songDetail.albumPic)
        } else {
            coverIcon = "\uf001"
            coverImage.source = ""
        }
        
        // 更新进度条
        currentTime = "0:00"
        totalTime = formattedDuration
        progressPercent = 0.0
        
        console.log("填充歌曲信息:", songName, "-", artistName, "专辑:", albumName)
    }

    // 连接 MusicService 的信号
    Connections {
        target: musicService
        function onSongDetailResult(songId, songDetail) {
            console.log("收到歌曲详情:", songId, songDetail)
            fillSongInfo(songDetail)
            // 获取播放 URL 并播放
            if (songDetail && songDetail.id) {
                musicService.getSongUrl(songId)
                // 同时获取歌词
                musicService.getLyric(songId)
            }
        }
        function onSongUrlResult(songId, url) {
            console.log("收到播放 URL:", url)
            if (url && url.length > 0) {
                player.source = url
                // 强制设置播放状态
                player.play()
                // 发送歌曲播放信号
                if (songName && artistName) {
                    console.log("发送歌曲播放信号:", songName, "by", artistName)
                    // 发送信号到父级
                    parent.songPlayed(songName, artistName)
                }
            }
        }
        function onLyricResult(songId, lyric) {
            console.log("收到歌词:", songId, "长度:", lyric.length)
            parseLyrics(lyric)
        }
        function onSearchResult(songs, hasMore) {
            playlistModel.clear()
            for (var i = 0; i < songs.length; i++) {
                playlistModel.append(songs[i])
            }
            console.log("搜索结果:", songs.length)
            if (songs.length > 0 && isAiTriggeredSearch) {
                console.log("AI触发的搜索，自动播放第一首歌曲:", songs[0].name, "- ", songs[0].artist)
                currentPlaylistIndex = 0
                if (musicService) {
                    musicService.getSongDetail(songs[0].id)
                }
                // 重置标志
                isAiTriggeredSearch = false
            } else if (songs.length > 0) {
                console.log("用户手动搜索，等待用户选择歌曲")
            }
        }
        function onErrorOccurred(message, details) {
            console.log("音乐服务错误:", message, details)
        }
    }
}