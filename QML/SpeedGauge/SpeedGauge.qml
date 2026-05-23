import QtQuick 2.15
import Qt5Compat.GraphicalEffects

Item {
    id: root

    // ========== 辅助函数：安全显示数字 ==========
    function safeSpeed() {
        if (root.speed === undefined || root.speed === null) return "--";
        return Math.round(root.speed);
    }
    function safeUnit() {
        return (root.unitText !== undefined && root.unitText !== null && root.unitText !== "") ? root.unitText : "--";
    }
    function safeMode() {
        return (root.modeText !== undefined && root.modeText !== null && root.modeText !== "") ? root.modeText : "--";
    }

    // ========== 动态数据属性（默认 undefined） ==========
    property var speed: undefined          // 车速
    property var unitText: undefined       // 单位文字
    property var modeText: undefined       // 模式文字

    // 固定配置属性（保留默认值）
    property int maxSpeed: 200
    property real totalAngle: 270
    property color backgroundColor: "#eef3fc"
    property color fillColor: "#2670e8"
    property color textColor: "#0c1c32"
    property color unitColor: "#577394"
    property color modeColor: "#2670e8"
    property int mainFontSize: 38
    property int unitFontSize: 14
    property int modeFontSize: 12

    implicitWidth: 180
    implicitHeight: 180

    // 背景圆环带阴影
    Rectangle {
        anchors.fill: parent
        anchors.margins: 5
        radius: width / 2
        color: "transparent"
        layer.enabled: true
        layer.effect: DropShadow {
            transparentBorder: true
            color: "#30000000"
            radius: 15
            samples: 31
            horizontalOffset: 0
            verticalOffset: 8
        }
    }

    // 背景圆环
    Rectangle {
        id: backgroundRing
        anchors.fill: parent
        anchors.margins: 5
        radius: width / 2
        gradient: Gradient {
            GradientStop { position: 0.0; color: "#ffffff" }
            GradientStop { position: 1.0; color: "#f5f9ff" }
        }
        border.color: "#d7e5fa"
        border.width: 1
    }

    Canvas {
        id: canvas
        anchors.fill: parent
        anchors.margins: 10
        onPaint: {
            var ctx = getContext("2d");
            ctx.reset();
            var w = width, h = height;
            var cx = w/2, cy = h/2;
            var r = Math.min(w, h)/2 - 5;

            // 灰色背景轨道圆环
            ctx.beginPath();
            ctx.arc(cx, cy, r, 0, Math.PI*2);
            ctx.lineWidth = 8;
            ctx.strokeStyle = root.backgroundColor;
            ctx.stroke();

            // 蓝色填充弧
            var speedVal = (root.speed !== undefined) ? root.speed : 0;
            var angle = (speedVal / root.maxSpeed) * root.totalAngle * (Math.PI/180);
            var startAngle = -Math.PI/2;
            var endAngle = startAngle + angle;

            ctx.beginPath();
            ctx.arc(cx, cy, r, startAngle, endAngle);
            ctx.lineWidth = 10;
            ctx.lineCap = "round";
            ctx.strokeStyle = root.fillColor;
            ctx.stroke();

            // 内圈装饰线
            ctx.beginPath();
            ctx.arc(cx, cy, r-15, 0, Math.PI*2);
            ctx.lineWidth = 1;
            ctx.strokeStyle = "#e0eaff";
            ctx.stroke();
        }
    }

    // 监听 root 属性变化，触发重绘
    Connections {
        target: root
        function onSpeedChanged() { canvas.requestPaint() }
        function onMaxSpeedChanged() { canvas.requestPaint() }
        function onTotalAngleChanged() { canvas.requestPaint() }
        function onBackgroundColorChanged() { canvas.requestPaint() }
        function onFillColorChanged() { canvas.requestPaint() }
    }

    // 中心内容区域
    Rectangle {
        id: centerCircle
        anchors.centerIn: parent
        width: parent.width * 0.55
        height: width
        radius: width / 2
        gradient: Gradient {
            GradientStop { position: 0.0; color: "#ffffff" }
            GradientStop { position: 1.0; color: "#f8fbff" }
        }
        border.color: "#e5f0ff"
        border.width: 1
        layer.enabled: true
        layer.effect: DropShadow {
            transparentBorder: true
            color: "#15000000"
            radius: 8
            samples: 21
            horizontalOffset: 0
            verticalOffset: 3
        }

        Column {
            anchors.centerIn: parent
            spacing: 1
            Text {
                anchors.horizontalCenter: parent.horizontalCenter
                text: safeSpeed()
                font.pixelSize: root.mainFontSize
                font.weight: Font.Bold
                color: root.textColor
            }
            Text {
                anchors.horizontalCenter: parent.horizontalCenter
                text: safeUnit()
                font.pixelSize: root.unitFontSize
                color: root.unitColor
            }
            Text {
                anchors.horizontalCenter: parent.horizontalCenter
                text: safeMode()
                font.pixelSize: root.modeFontSize
                font.weight: Font.Bold
                color: root.modeColor
            }
        }
    }
}
