import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15
import Qt5Compat.GraphicalEffects

Item {
    id: aiPanelRoot
    FontLoader { id: solidFont; source: "qrc:/QML/fonts/fa-solid-900.ttf" }

    function markdownToHtml(markdown) {
        var html = markdown
        html = html.replace(/&/g, "&amp;")
        html = html.replace(/</g, "&lt;")
        html = html.replace(/>/g, "&gt;")
        html = html.replace(/```([\s\S]*?)```/g, function(match, code) {
            return "<pre style='background-color: #2d2d2d; color: #f8f8f2; padding: 12px; border-radius: 8px; overflow-x: auto; font-family: monospace;'>" + code.trim() + "</pre>"
        })
        html = html.replace(/\*\*(.*?)\*\*/g, "<b>$1</b>")
        html = html.replace(/\*(.*?)\*/g, "<i>$1</i>")
        html = html.replace(/`(.*?)`/g, "<code style='background-color: #e0e0e0; padding: 2px 4px; border-radius: 3px; font-family: monospace;'>$1</code>")
        html = html.replace(/^### (.*?)$/gm, "<h3 style='margin-top: 8px; margin-bottom: 4px;'>$1</h3>")
        html = html.replace(/^## (.*?)$/gm, "<h2 style='margin-top: 10px; margin-bottom: 5px;'>$1</h2>")
        html = html.replace(/^# (.*?)$/gm, "<h1 style='margin-top: 12px; margin-bottom: 6px;'>$1</h1>")
        html = html.replace(/^\> (.*?)$/gm, "<blockquote style='border-left: 3px solid #2670e8; padding-left: 10px; margin-left: 0; color: #666;'>$1</blockquote>")
        html = html.replace(/\n/g, "<br>")
        return html
    }

    // ── Audio ──
    Connections {
        target: audioService
        function onTranscriptionResult(text) { inputText = text }
    }

    // ── AgentLoop signals ──
    Connections {
        target: agentLoop

        function onThinkingStarted() {
            isTyping = true
            agentStepText = ""
            currentAiContent = ""
            taskList.clear()
            tasklistIndex = -1
        }

        function onThinkingFinished() {
            isTyping = false
            agentStepText = ""
            streamIndex = -1
            currentAiContent = ""
            streamTimestamp = ""
            // Keep tasklist bubble as history, don't remove it
            tasklistIndex = -1
            isAtBottom = true
            chatListView.positionViewAtEnd()
        }

        function onStreamingDelta(delta) {
            currentAiContent += delta
            if (streamIndex >= 0) {
                chatMessages.set(streamIndex, { type: "ai", message: currentAiContent, timestamp: streamTimestamp })
            } else {
                var now = new Date()
                streamTimestamp = now.getHours() + ":" + (now.getMinutes() < 10 ? "0" : "") + now.getMinutes()
                chatMessages.append({ type: "ai", message: currentAiContent, timestamp: streamTimestamp })
                streamIndex = chatMessages.count - 1
            }
            isAtBottom = true
            chatListView.positionViewAtEnd()
        }

        function onStepUpdate(toolName, step, totalSteps) {
            agentStepText = "正在执行: " + toolName + " (" + step + "/" + totalSteps + ")"
        }

        function onTaskProgress(taskName, status, step, totalSteps) {
            // Ensure tasklist bubble exists in chat
            if (tasklistIndex < 0 && taskList.count === 0) {
                chatMessages.append({ type: "tasklist" })
                tasklistIndex = chatMessages.count - 1
            }
            // Update taskList
            var found = false
            for (var i = 0; i < taskList.count; i++) {
                if (taskList.get(i).name === taskName) {
                    taskList.set(i, { name: taskName, status: status, step: step, total: totalSteps })
                    found = true
                    break
                }
            }
            if (!found) {
                taskList.append({ name: taskName, status: status, step: step, total: totalSteps })
            }
            // Force the tasklist delegate to re-evaluate by touching the entry
            if (tasklistIndex >= 0 && tasklistIndex < chatMessages.count) {
                chatMessages.set(tasklistIndex, { type: "tasklist", _touch: Date.now() })
            }
        }

        function onErrorOccurred(error) {
            isTyping = false
            agentStepText = ""
            var now = new Date()
            var timeStr = now.getHours() + ":" + (now.getMinutes() < 10 ? "0" : "") + now.getMinutes()
            chatMessages.append({ type: "ai", message: "错误: " + error, timestamp: timeStr })
            isAtBottom = true
            chatListView.positionViewAtEnd()
        }
    }

    property int streamIndex: -1
    property int tasklistIndex: -1
    property string streamTimestamp: ""
    property string currentAiContent: ""
    property string agentStepText: ""

    property var chatMessages: ListModel {
        ListElement { type: "ai"; message: "你好！我是车载智能助手，有什么可以帮你的吗？"; timestamp: "19:45" }
    }

    property var taskList: ListModel { }

    property bool isTyping: false
    property string inputText: ""
    property real recordingTime: 0.0
    property bool isRecording: false
    property bool showOverlay: false
    property bool isAtBottom: true

    signal startRecordingSignal()
    signal stopRecordingSignal()

    Timer {
        id: recordingTimer
        interval: 100; repeat: true
        onTriggered: {
            recordingTime += 0.1
            if (recordingTime >= 20.0) {
                voiceBtn.scale = 1.0; voiceBtn.z = 1
                recordingTimer.stop()
                showOverlay = false
                audioService.stopRecording()
                aiPanelRoot.stopRecordingSignal()
            }
        }
    }

    onShowOverlayChanged: {
        if (showOverlay) {
            recordingTime = 0.0
            recordingTimer.start()
            aiPanelRoot.startRecordingSignal()
        } else {
            recordingTimer.stop()
            aiPanelRoot.stopRecordingSignal()
        }
    }

    Rectangle {
        anchors.fill: parent; color: "#ffffff"; radius: 20

        ColumnLayout {
            anchors.fill: parent; anchors.margins: 0; spacing: 0

            // ── Header ──
            Rectangle {
                Layout.fillWidth: true; height: 56; color: "#2670e8"; radius: 20
                RowLayout {
                    anchors.fill: parent; anchors.leftMargin: 20; anchors.rightMargin: 16; spacing: 12
                    Rectangle { width: 40; height: 40; radius: 20; color: "#ffffff"
                        Text { anchors.centerIn: parent; text: ""; font.family: solidFont.name; font.pixelSize: 20; color: "#2670e8" }
                    }
                    ColumnLayout { spacing: 2; Layout.fillWidth: true
                        Text { text: "智能助手"; font.pixelSize: 18; font.weight: Font.Bold; color: "#ffffff" }
                        Text { text: "在线"; font.pixelSize: 12; color: "#ffffff" }
                    }
                    Item { Layout.fillWidth: true }
                    Rectangle { width: 40; height: 40; radius: 20; color: "#ffffff"; opacity: 0.8
                        Text { anchors.centerIn: parent; text: ""; font.family: solidFont.name; font.pixelSize: 16; color: "#2670e8" }
                        MouseArea {
                            anchors.fill: parent
                            onClicked: {
                                agentLoop.clearMemory()
                                chatMessages.clear()
                                taskList.clear()
                                tasklistIndex = -1
                                chatMessages.append({ type: "ai", message: "你好！我是车载智能助手，有什么可以帮你的吗？", timestamp: "" })
                            }
                        }
                    }
                    Rectangle { width: 40; height: 40; radius: 20; color: "#ffffff"; opacity: 0.8
                        Text { anchors.centerIn: parent; text: ""; font.family: solidFont.name; font.pixelSize: 18; color: "#2670e8" }
                        MouseArea {
                            anchors.fill: parent
                            onClicked: {
                                root.color = (root.color === "#1e2f47") ? "#ffffff" : "#1e2f47"
                            }
                        }
                    }
                }
            }

            // ── Chat area ──
            Rectangle {
                Layout.fillWidth: true; Layout.fillHeight: true; color: "#ffffff"; radius: 20

                ListView {
                    id: chatListView; anchors.fill: parent; anchors.margins: 16; spacing: 12; clip: true; model: chatMessages

                    onMovementEnded: checkIfAtBottom()
                    onFlickEnded: checkIfAtBottom()

                    function checkIfAtBottom() {
                        isAtBottom = (atYEnd || contentY + height >= contentHeight)
                    }

                    delegate: Item {
                        id: delegateRoot
                        width: chatListView.width
                        height: model.type === "tasklist" ? (taskPart.height) : (chatContentContainer.height + 20)

                        // ── Chat bubble (ai / user) ──
                        Item {
                            id: chatPart
                            visible: model.type !== "tasklist"
                            width: parent.width
                            height: chatContentContainer.height + 20

                            Rectangle {
                                anchors.top: parent.top; width: 44; height: 40; visible: model.type === "ai"
                                Rectangle {
                                    anchors.top: parent.top; anchors.left: parent.left
                                    width: 40; height: 40; radius: 20; color: "#2670e8"
                                    Text { anchors.centerIn: parent; text: ""; font.family: solidFont.name; font.pixelSize: 18; color: "#ffffff" }
                                }
                            }

                            Column {
                                id: chatContentContainer
                                anchors.top: parent.top
                                anchors.left: model.type === "ai" ? parent.left : undefined
                                anchors.right: model.type === "user" ? parent.right : undefined
                                anchors.leftMargin: model.type === "ai" ? 56 : 0
                                anchors.rightMargin: model.type === "user" ? 56 : 0
                                spacing: 4

                                Rectangle {
                                    color: model.type === "ai" ? "#f0f6ff" : "#2670e8"
                                    radius: 20
                                    width: Math.min(chatListView.width * 0.9, chatText.implicitWidth + 32)
                                    height: chatText.implicitHeight + 32
                                    Text {
                                        id: chatText
                                        anchors.left: parent.left; anchors.right: parent.right
                                        anchors.top: parent.top
                                        anchors.leftMargin: 16; anchors.rightMargin: 16; anchors.topMargin: 16
                                        text: model.type === "ai" ? markdownToHtml(model.message || "") : (model.message || "")
                                        font.pixelSize: 15
                                        color: model.type === "ai" ? "#1e2f47" : "#ffffff"
                                        wrapMode: Text.WordWrap
                                        textFormat: model.type === "ai" ? Text.RichText : Text.PlainText
                                        width: parent.width - 32
                                    }
                                }

                                Text {
                                    text: model.timestamp || ""
                                    font.pixelSize: 11; color: "#8ba3c2"
                                    anchors.left: model.type === "ai" ? parent.left : undefined
                                    anchors.right: model.type === "user" ? parent.right : undefined
                                }
                            }

                            Rectangle {
                                anchors.top: parent.top; width: 44; height: 40; anchors.right: parent.right; visible: model.type === "user"
                                Rectangle {
                                    anchors.top: parent.top; anchors.right: parent.right
                                    width: 40; height: 40; radius: 20; color: "#4caf50"
                                    Text { anchors.centerIn: parent; text: ""; font.family: solidFont.name; font.pixelSize: 18; color: "#ffffff" }
                                }
                            }
                        }

                        // ── Task list bubble ──
                        Item {
                            id: taskPart
                            visible: model.type === "tasklist"
                            width: parent.width
                            height: taskListColumn.implicitHeight + 24

                            Rectangle {
                                anchors.left: parent.left; anchors.leftMargin: 56
                                width: Math.min(chatListView.width * 0.75, 280)
                                height: taskListColumn.implicitHeight + 20
                                color: "#fafbfc"
                                radius: 16
                                border.color: "#e8ecf0"
                                border.width: 1

                                Column {
                                    id: taskListColumn
                                    anchors.left: parent.left; anchors.right: parent.right
                                    anchors.top: parent.top
                                    anchors.margins: 10
                                    spacing: 4

                                    Repeater {
                                        model: taskList
                                        RowLayout {
                                            width: taskListColumn.width
                                            spacing: 8
                                            Text {
                                                text: model.status === "running" ? "" :
                                                      model.status === "done"    ? "" :
                                                      model.status === "error"   ? "" : ""
                                                font.family: solidFont.name
                                                font.pixelSize: 13
                                                color: model.status === "running" ? "#f0a030" :
                                                       model.status === "done"    ? "#22a85d" :
                                                       model.status === "error"   ? "#e04040" : "#ccc"
                                            }
                                            Text {
                                                text: model.name
                                                font.pixelSize: 13
                                                color: model.status === "done"    ? "#888" :
                                                       model.status === "running" ? "#1e2f47" : "#aaa"
                                            }
                                            Item { Layout.fillWidth: true }
                                            Text {
                                                text: model.step + "/" + model.total
                                                font.pixelSize: 11
                                                color: "#ccc"
                                                visible: model.status !== "done"
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                    Component.onCompleted: positionViewAtEnd()
                }

                // ── Thinking indicator ──
                Rectangle {
                    anchors.left: parent.left; anchors.bottom: parent.bottom
                    anchors.leftMargin: 72; anchors.bottomMargin: 16
                    visible: isTyping
                    color: "#f0f6ff"; radius: 20
                    width: thinkingRow.implicitWidth + 24; height: 40

                    Row {
                        id: thinkingRow
                        anchors.centerIn: parent; spacing: 4
                        Rectangle { width: 8; height: 8; radius: 4; color: "#2670e8"
                            SequentialAnimation on opacity {
                                loops: Animation.Infinite
                                NumberAnimation { from: 0.3; to: 1.0; duration: 500 }
                                NumberAnimation { from: 1.0; to: 0.3; duration: 500 }
                            }
                        }
                        Rectangle { width: 8; height: 8; radius: 4; color: "#2670e8"; opacity: 0.3
                            SequentialAnimation on opacity {
                                loops: Animation.Infinite
                                PauseAnimation { duration: 150 }
                                NumberAnimation { from: 0.3; to: 1.0; duration: 500 }
                                NumberAnimation { from: 1.0; to: 0.3; duration: 500 }
                            }
                        }
                        Rectangle { width: 8; height: 8; radius: 4; color: "#2670e8"; opacity: 0.3
                            SequentialAnimation on opacity {
                                loops: Animation.Infinite
                                PauseAnimation { duration: 300 }
                                NumberAnimation { from: 0.3; to: 1.0; duration: 500 }
                                NumberAnimation { from: 1.0; to: 0.3; duration: 500 }
                            }
                        }
                    }
                }

                // ── Step status ──
                Text {
                    anchors.left: parent.left; anchors.leftMargin: 16
                    anchors.bottom: parent.bottom; anchors.bottomMargin: 8
                    text: agentStepText; font.pixelSize: 12; color: "#8ba3c2"; visible: agentStepText !== ""
                }

                // ── Scroll-to-bottom ──
                Rectangle {
                    anchors.right: parent.right; anchors.bottom: parent.bottom
                    anchors.bottomMargin: 24; anchors.rightMargin: 24
                    visible: !isAtBottom; z: 10
                    Rectangle {
                        width: 40; height: 40; radius: 20; color: "#ffffff"
                        border.color: "#cfe0f5"; border.width: 1
                        Text { anchors.centerIn: parent; text: ""; font.family: solidFont.name; font.pixelSize: 18; color: "#2670e8" }
                        MouseArea {
                            anchors.fill: parent; hoverEnabled: true; cursorShape: Qt.PointingHandCursor
                            onEntered: parent.color = "#f0f6ff"
                            onExited: parent.color = "#ffffff"
                            onClicked: { chatListView.positionViewAtEnd(); isAtBottom = true }
                        }
                    }
                }
            }

            // ── Input ──
            Rectangle {
                id: inputContainer
                Layout.fillWidth: true; height: 60; color: "#f0f6ff"; radius: 20
                RowLayout {
                    anchors.fill: parent; anchors.leftMargin: 16; anchors.rightMargin: 8; spacing: 12
                    Rectangle {
                        id: voiceBtn
                        width: 44; height: 44; radius: 22; color: "#ffffff"; border.color: "#cfe0f5"; border.width: 1
                        Text { anchors.centerIn: parent; text: ""; font.family: solidFont.name; font.pixelSize: 20; color: "#2670e8" }
                        MouseArea {
                            anchors.fill: parent
                            onPressed: { voiceBtn.scale = 1.5; voiceBtn.z = 2; showOverlay = true; audioService.startRecording() }
                            onReleased: { voiceBtn.scale = 1.0; voiceBtn.z = 1; showOverlay = false; audioService.stopRecording() }
                        }
                        Behavior on scale { NumberAnimation { duration: 80; easing.type: Easing.OutQuad } }
                    }
                    TextField {
                        id: inputField; Layout.fillWidth: true; placeholderText: "输入消息..."
                        text: inputText; onTextChanged: inputText = text; background: null
                        font.pixelSize: 15; color: "#1e2f47"; selectByMouse: true; onAccepted: sendMessage()
                    }
                    Rectangle {
                        id: sendBtn; width: 44; height: 44; radius: 22; color: "#2670e8"
                        Text { anchors.centerIn: parent; text: ""; font.family: solidFont.name; font.pixelSize: 18; color: "#ffffff" }
                        MouseArea { anchors.fill: parent; onClicked: sendMessage() }
                    }
                }
            }
        }
    }

    // (delegates inlined above — old Components removed)

    function sendMessage() {
        if (inputText.trim() === "") return
        var now = new Date()
        var timeStr = now.getHours() + ":" + (now.getMinutes() < 10 ? "0" : "") + now.getMinutes()
        chatMessages.append({ type: "user", message: inputText.trim(), timestamp: timeStr })
        var messageText = inputText.trim()
        inputText = ""
        streamIndex = -1
        currentAiContent = ""
        tasklistIndex = -1
        taskList.clear()
        isAtBottom = true
        chatListView.positionViewAtEnd()
        agentLoop.run(messageText)
    }
}
