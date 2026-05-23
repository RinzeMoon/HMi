import QtQuick 2.15
import QtQuick.Layouts 1.15

Item {
    id: root
    property int activeIndex: 0
    signal sidebarClicked(int index)

    property color activeBgColor: "#2670e8"
    property color hoverBgColor: "#e0edff"
    property color inactiveIconColor: "#2c4b74"
    property color hoverIconColor: "#2670e8"
    property int iconSize: 24
    property int iconRectSize: 56
    property int spacing: 20

    property var iconTexts: ["\uf625", "\uf5a0", "\uf1b9", "\uf001", "\uf544"]

    FontLoader { id: solidFont; source: "qrc:/QML/fonts/fa-solid-900.ttf" }

    Rectangle {
        anchors.fill: parent
        color: "#f6faff"
        radius: 40
        border.color: "#dde9fc"
        border.width: 1
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.topMargin: 20
        anchors.bottomMargin: 20
        anchors.leftMargin: 12
        anchors.rightMargin: 12
        spacing: root.spacing

        Rectangle {
            id: icon1
            Layout.preferredWidth: root.iconRectSize
            Layout.preferredHeight: root.iconRectSize
            radius: root.iconRectSize / 2
            color: root.activeIndex === 0 ? root.activeBgColor : (hovered ? root.hoverBgColor : "transparent")
            Behavior on color { ColorAnimation { duration: 150 } }

            property bool hovered: false

            Text {
                anchors.centerIn: parent
                text: iconTexts[0]
                font.family: solidFont.name
                font.pixelSize: root.iconSize
                color: root.activeIndex === 0 ? "white" : (icon1.hovered ? root.hoverIconColor : root.inactiveIconColor)
            }

            MouseArea {
                anchors.fill: parent
                hoverEnabled: true
                onEntered: icon1.hovered = true
                onExited: icon1.hovered = false
                onClicked: root.sidebarClicked(0)
            }
        }

        Rectangle {
            id: icon2
            Layout.preferredWidth: root.iconRectSize
            Layout.preferredHeight: root.iconRectSize
            radius: root.iconRectSize / 2
            color: root.activeIndex === 1 ? root.activeBgColor : (hovered ? root.hoverBgColor : "transparent")

            property bool hovered: false

            Text {
                anchors.centerIn: parent
                text: iconTexts[1]
                font.family: solidFont.name
                font.pixelSize: root.iconSize
                color: root.activeIndex === 1 ? "white" : (icon2.hovered ? root.hoverIconColor : root.inactiveIconColor)
            }

            MouseArea {
                anchors.fill: parent
                hoverEnabled: true
                onEntered: icon2.hovered = true
                onExited: icon2.hovered = false
                onClicked: root.sidebarClicked(1)
            }
        }

        Rectangle {
            id: icon3
            Layout.preferredWidth: root.iconRectSize
            Layout.preferredHeight: root.iconRectSize
            radius: root.iconRectSize / 2
            color: root.activeIndex === 2 ? root.activeBgColor : (hovered ? root.hoverBgColor : "transparent")

            property bool hovered: false

            Text {
                anchors.centerIn: parent
                text: iconTexts[2]
                font.family: solidFont.name
                font.pixelSize: root.iconSize
                color: root.activeIndex === 2 ? "white" : (icon3.hovered ? root.hoverIconColor : root.inactiveIconColor)
            }

            MouseArea {
                anchors.fill: parent
                hoverEnabled: true
                onEntered: icon3.hovered = true
                onExited: icon3.hovered = false
                onClicked: root.sidebarClicked(2)
            }
        }

        Rectangle {
            id: icon4
            Layout.preferredWidth: root.iconRectSize
            Layout.preferredHeight: root.iconRectSize
            radius: root.iconRectSize / 2
            color: root.activeIndex === 3 ? root.activeBgColor : (hovered ? root.hoverBgColor : "transparent")

            property bool hovered: false

            Text {
                anchors.centerIn: parent
                text: iconTexts[3]
                font.family: solidFont.name
                font.pixelSize: root.iconSize
                color: root.activeIndex === 3 ? "white" : (icon4.hovered ? root.hoverIconColor : root.inactiveIconColor)
            }

            MouseArea {
                anchors.fill: parent
                hoverEnabled: true
                onEntered: icon4.hovered = true
                onExited: icon4.hovered = false
                onClicked: root.sidebarClicked(3)
            }
        }

        Rectangle {
            id: icon5
            Layout.preferredWidth: root.iconRectSize
            Layout.preferredHeight: root.iconRectSize
            radius: root.iconRectSize / 2
            color: root.activeIndex === 4 ? root.activeBgColor : (hovered ? root.hoverBgColor : "transparent")

            property bool hovered: false

            Text {
                anchors.centerIn: parent
                text: iconTexts[4]
                font.family: solidFont.name
                font.pixelSize: root.iconSize
                color: root.activeIndex === 4 ? "white" : (icon5.hovered ? root.hoverIconColor : root.inactiveIconColor)
            }

            MouseArea {
                anchors.fill: parent
                hoverEnabled: true
                onEntered: icon5.hovered = true
                onExited: icon5.hovered = false
                onClicked: root.sidebarClicked(4)
            }
        }

        Item { Layout.fillHeight: true }
    }
}
