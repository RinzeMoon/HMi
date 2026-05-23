import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Window
import com.carhmi.opengl 1.0

Rectangle {
    id: root
    color: "#1a2742"

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        Item {
            Layout.fillWidth: true
            Layout.minimumHeight: 40
            Layout.maximumHeight: 40

            Text {
                anchors.centerIn: parent
                text: "3D 展示面板"
                font.pixelSize: 18
                font.weight: Font.Bold
                color: "#ffffff"
            }
        }

        Item {
            Layout.fillWidth: true
            Layout.minimumHeight: 5
            Layout.maximumHeight: 5
        }

        Rectangle {
            id: glContainer
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.minimumWidth: 640
            Layout.minimumHeight: 480
            Layout.preferredWidth: 800
            Layout.preferredHeight: 600
            color: "#0d1520"
            border.color: "#334155"
            border.width: 1

            CarQt3DView {
                id: glItem
                anchors.fill: parent
                autoRotate: true
                modelPath: "file:///d:/QtWorks/CarHMI/porsche_911_gt3.glb"

                Component.onCompleted: {
                    console.log("CarQt3DView completed, size:", width, "x", height)
                }
            }
        }

        Item {
            Layout.fillWidth: true
            Layout.minimumHeight: 5
            Layout.maximumHeight: 5
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.maximumHeight: 50
            spacing: 12

            Rectangle {
                Layout.fillWidth: true
                Layout.minimumHeight: 40
                height: 40
                radius: 20
                color: glItem.autoRotate ? "#2670e8" : "#f0f6ff"
                border.color: "#2670e8"
                border.width: 1

                Text {
                    anchors.centerIn: parent
                    text: glItem.autoRotate ? "自动旋转中" : "已暂停"
                    font.pixelSize: 14
                    font.weight: Font.Bold
                    color: glItem.autoRotate ? "white" : "#2670e8"
                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        glItem.autoRotate = !glItem.autoRotate
                    }
                }
            }

            Rectangle {
                Layout.minimumWidth: 100
                Layout.minimumHeight: 40
                height: 40
                radius: 20
                color: "#334155"
                border.color: "#475569"
                border.width: 1

                Text {
                    anchors.centerIn: parent
                    text: "加载模型"
                    font.pixelSize: 14
                    color: "#ffffff"
                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        console.log("Load model clicked")
                    }
                }
            }
        }

        Item {
            Layout.fillWidth: true
            Layout.minimumHeight: 10
            Layout.maximumHeight: 10
        }
    }
}