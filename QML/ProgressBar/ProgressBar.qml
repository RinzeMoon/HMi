import QtQuick 2.15
import QtQuick.Layouts 1.15

Item {
    property string label: ""
    property int value: 0
    property int max: 100
    property color barColor: "#2670e8"

    implicitHeight: 40

    ColumnLayout {
        anchors.fill: parent
        spacing: 4

        RowLayout {
            Layout.fillWidth: true
            Text {
                text: label
                font.pixelSize: 14
                color: "#2f405a"
            }
            Item { Layout.fillWidth: true }
            Text {
                text: value + " / " + max
                font.pixelSize: 14
                color: "#2f405a"
            }
        }

        Rectangle {
            Layout.fillWidth: true
            height: 8
            color: "#dde5ef"
            radius: 4
            Rectangle {
                width: parent.width * (value / max)
                height: parent.height
                color: barColor
                radius: 4
            }
        }
    }
}