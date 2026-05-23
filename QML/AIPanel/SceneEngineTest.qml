import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15

Rectangle {
    id: testPanel
    width: 400
    height: 600
    color: "#f5f5f5"
    radius: 10
    
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 12
        
        Text {
            text: "SceneEngine 测试面板"
            font.pixelSize: 20
            font.bold: true
            color: "#1e2f47"
        }
        
        Rectangle {
            Layout.fillWidth: true
            height: 1
            color: "#cfe0f5"
        }
        
        TextField {
            id: inputField
            Layout.fillWidth: true
            placeholderText: "输入测试语句..."
            font.pixelSize: 14
        }
        
        RowLayout {
            Layout.fillWidth: true
            spacing: 8
            
            Button {
                Layout.fillWidth: true
                text: "处理输入"
                onClicked: {
                    if (inputField.text.trim() !== "") {
                        sceneEngine.processUserInput(inputField.text)
                        logMessage("用户输入: " + inputField.text)
                        inputField.text = ""
                    }
                }
            }
        }
        
        Text {
            text: "测试场景:"
            font.pixelSize: 14
            font.bold: true
            color: "#1e2f47"
        }
        
        RowLayout {
            Layout.fillWidth: true
            spacing: 8
            
            Button {
                Layout.fillWidth: true
                text: "调低空调 (高置信度)"
                onClicked: {
                    sceneEngine.processUserInput("调低空调温度")
                    logMessage("测试: 调低空调温度 (高置信度)")
                }
            }
        }
        
        RowLayout {
            Layout.fillWidth: true
            spacing: 8
            
            Button {
                Layout.fillWidth: true
                text: "可能要调空调 (中置信度)"
                onClicked: {
                    sceneEngine.processUserInput("好像有点热")
                    logMessage("测试: 好像有点热 (中置信度)")
                }
            }
        }
        
        RowLayout {
            Layout.fillWidth: true
            spacing: 8
            
            Button {
                Layout.fillWidth: true
                text: "模糊输入 (低置信度)"
                onClicked: {
                    sceneEngine.processUserInput("那个什么来着")
                    logMessage("测试: 那个什么来着 (低置信度)")
                }
            }
        }
        
        Text {
            text: "确认/澄清操作:"
            font.pixelSize: 14
            font.bold: true
            color: "#1e2f47"
        }
        
        RowLayout {
            Layout.fillWidth: true
            spacing: 8
            
            Button {
                Layout.fillWidth: true
                text: "确认"
                onClicked: {
                    sceneEngine.handleUserConfirmation(true)
                    logMessage("用户: 确认")
                }
            }
            
            Button {
                Layout.fillWidth: true
                text: "取消"
                onClicked: {
                    sceneEngine.handleUserConfirmation(false)
                    logMessage("用户: 取消")
                }
            }
        }
        
        Text {
            text: "日志输出:"
            font.pixelSize: 14
            font.bold: true
            color: "#1e2f47"
        }
        
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: "#ffffff"
            radius: 8
            border.color: "#cfe0f5"
            border.width: 1
            
            ScrollView {
                anchors.fill: parent
                anchors.margins: 8
                
                TextArea {
                    id: logArea
                    readOnly: true
                    font.pixelSize: 12
                    color: "#1e2f47"
                    placeholderText: "等待测试..."
                }
            }
        }
    }
    
    Connections {
        target: sceneEngine
        
        function onAiResponseGenerated(response) {
            logMessage("AI响应: " + response)
        }
        
        function onConfirmationRequested(message) {
            logMessage("请求确认: " + message)
        }
        
        function onClarificationRequested(message) {
            logMessage("请求澄清: " + message)
        }
        
        function onActionExecuted(actionId, success, message) {
            logMessage("动作执行: " + actionId + " - " + (success ? "成功" : "失败") + " - " + message)
        }
    }
    
    function logMessage(msg) {
        var now = new Date()
        var timeStr = now.getHours().toString().padStart(2, '0') + ":" + 
                      now.getMinutes().toString().padStart(2, '0') + ":" + 
                      now.getSeconds().toString().padStart(2, '0')
        logArea.append("[" + timeStr + "] " + msg)
    }
}
