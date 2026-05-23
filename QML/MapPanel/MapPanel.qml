import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Basic 2.15 as Basic
import Qt5Compat.GraphicalEffects
import QtWebEngine 1.15

Item {
    FontLoader { id: solidFont; source: "qrc:/QML/fonts/fa-solid-900.ttf" }

    Timer {
        id: initTimer
        interval: 10
        repeat: false
        running: true
        onTriggered: {
            panelY = parent.height;
            console.log("[MapPanel] 初始化面板位置，y =", panelY);
        }
    }

    property var title: "地图导航"
    property string startAddress: "北京西站"
    property string endAddress: "天安门广场"
    property string currentCity: "北京"
    property int currentPolicy: 0
    property bool isSearching: false
    property bool panelOpen: false
    property real panelY: 9999
    property string currentStartLoc: ""
    property string currentEndLoc: ""
    property real lastCenterLng: 0
    property real lastCenterLat: 0
    
    readonly property var cityList: ["北京", "上海", "广州", "深圳", "天津", "重庆", "成都", "杭州", "武汉", "西安", "南京", "苏州", "长春", "沈阳", "哈尔滨", "大连", "青岛", "济南", "郑州", "长沙", "合肥", "福州", "厦门", "南昌", "南宁", "海口", "昆明", "贵阳", "兰州", "西宁", "银川", "乌鲁木齐", "呼和浩特", "太原", "石家庄"]
    
    Connections {
        target: locationService
        onPositionUpdated: {
            var dist = Math.abs(longitude - lastCenterLng) + Math.abs(latitude - lastCenterLat);
            if (dist > 0.002 || (lastCenterLng === 0 && lastCenterLat === 0)) {
                console.log("[MapPanel] 位置更新:", latitude, longitude, "移动:", (dist * 111000).toFixed(0), "m");
                lastCenterLng = longitude;
                lastCenterLat = latitude;
                setMapCenter(longitude, latitude, 15);
            }
        }
    }
    
    function setMapCenter(lng, lat, zoom) {
        console.log("[MapPanel] 设置地图中心:", lng, lat, zoom);
        var js = "if (window.qmlBridge) { window.qmlBridge.setCenter(" + 
                 lng + ", " + lat + ", " + zoom + "); }";
        if (backgroundWebView && backgroundWebView.loadProgress === 100) {
            backgroundWebView.runJavaScript(js);
        }
    }

    function closePanel() {
        console.log("[MapPanel] 关闭面板");
        panelOpen = false;
        panelY = parent.height;
    }

    function updateMapRoute() {
        console.log("[MapPanel] updateMapRoute called");
        if (!currentStartLoc || !currentEndLoc) return;
        
        var js = "if (window.qmlBridge) { window.qmlBridge.showRoute(" + 
                 JSON.stringify(currentStartLoc) + ", " + 
                 JSON.stringify(currentEndLoc) + ", ''); }";
        
        console.log("[MapPanel] Calling JS");
        
        if (backgroundWebView && backgroundWebView.loadProgress === 100) {
            backgroundWebView.runJavaScript(js);
        }
    }
    
    function openPanelAndNavigate(startAddr, endAddr, city) {
        console.log("[MapPanel] openPanelAndNavigate called:", startAddr, endAddr, city);
        if (startAddr) startAddress = startAddr;
        if (endAddr) endAddress = endAddr;
        if (city) currentCity = city;
        
        panelOpen = true;
        panelY = 0;
        
        console.log("[MapPanel] Opening panel and starting search");
        isSearching = true;
        mapService.searchRoute(startAddress, endAddress, "driving", currentPolicy, currentCity);
    }

    // 背景区域（圆角）- 显示地图
    Rectangle {
        anchors.fill: parent
        anchors.margins: 16
        color: "#e9f2fe"
        radius: 24
        clip: true

        // 左上角文字
        RowLayout {
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.topMargin: 20
            anchors.leftMargin: 24
            z: 10
            spacing: 8

            Text {
                text: ""
                font.family: solidFont.name
                font.pixelSize: 24
                color: "#2670e8"
            }

            Text {
                text: title
                font.pixelSize: 22
                font.weight: Font.Bold
                color: "#0c1c32"
            }
        }

        WebEngineView {
            id: backgroundWebView
            anchors.fill: parent
            anchors.topMargin: 60
            anchors.margins: 4
            url: "qrc:/QML/MapPanel/map.html"
            
            onLoadProgressChanged: {
                if (loadProgress === 100) {
                    console.log("[MapPanel] 背景地图页面加载成功，正在初始化...");
                    var apiKey = mapService.getJsApiKey();
                    var securityKey = mapService.getSecurityKey();
                    console.log("[MapPanel] Using JS API Key:", apiKey ? apiKey.substring(0, 8) + "..." : "NONE");
                    console.log("[MapPanel] Using Security Key:", securityKey ? securityKey.substring(0, 8) + "..." : "NONE");
                    var initJs = "if (window.qmlBridge) { window.qmlBridge.init(" + 
                                 JSON.stringify(apiKey) + ", " + 
                                 JSON.stringify(securityKey) + "); }";
                    backgroundWebView.runJavaScript(initJs);
                    
                    console.log("[MapPanel] 启动定位服务...");
                    locationService.startLocation();
                }
            }
        }
    }

    // 底部触发按钮
    Rectangle {
        id: openButton
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottomMargin: 32
        width: 64
        height: 64
        color: "#2670e8"
        radius: 32
        opacity: panelOpen ? 0 : 0.95
        visible: !panelOpen

        Text {
            anchors.centerIn: parent
            text: ""
            font.family: solidFont.name
            font.pixelSize: 26
            color: "#ffffff"
        }

        MouseArea {
            anchors.fill: parent
            onClicked: {
                panelOpen = true;
                panelY = 0;
                console.log("[MapPanel] 打开面板");
            }
        }

        Behavior on opacity { NumberAnimation { duration: 200 } }
    }

    // 主面板
    Rectangle {
        id: mainPanel
        anchors.left: parent.left
        anchors.right: parent.right
        y: panelY
        height: parent.height
        color: "#ffffff"

        // 顶部栏（带关闭按钮）
        Rectangle {
            id: topBar
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            height: 60
            color: "#ffffff"

            // 关闭按钮
            Rectangle {
                id: closeButton
                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter
                anchors.rightMargin: 16
                width: 40
                height: 40
                color: "#f8fafc"
                radius: 20

                Text {
                    anchors.centerIn: parent
                    text: ""
                    font.family: solidFont.name
                    font.pixelSize: 18
                    color: "#8ba3c2"
                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        closePanel();
                    }
                }
            }

            // 标题
            Text {
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.bottom: parent.bottom
                anchors.bottomMargin: 10
                text: "路线规划"
                font.pixelSize: 18
                font.weight: Font.Bold
                color: "#0c1c32"
            }

            // 把手装饰
            Rectangle {
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.top: parent.top
                anchors.topMargin: 8
                width: 40
                height: 5
                color: "#d0ddee"
                radius: 2.5
            }
        }

        // 面板内容
        RowLayout {
            anchors.top: topBar.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            anchors.margins: 16
            spacing: 16

            // 左侧：路线规划选项
            Rectangle {
                Layout.preferredWidth: 340
                Layout.fillHeight: true
                color: "#f8fafc"
                radius: 16
                border.color: "#e2e8f0"
                border.width: 1

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 20
                    spacing: 16

                    Text {
                        text: "驾车导航"
                        font.pixelSize: 18
                        font.weight: Font.Bold
                        color: "#2670e8"
                    }

                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 6

                        Text {
                            text: "城市"
                            font.pixelSize: 13
                            color: "#555"
                        }

                        Basic.ComboBox {
                            id: citySelect
                            Layout.fillWidth: true
                            Layout.preferredHeight: 42
                            model: cityList
                            currentIndex: cityList.indexOf(currentCity)
                            onActivated: {
                                currentCity = cityList[index];
                                console.log("[MapPanel] 城市已选择:", currentCity);
                            }
                            background: Rectangle {
                                color: "#ffffff"
                                border.color: "#ddd"
                                radius: 8
                            }
                        }
                    }

                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 6

                        Text {
                            text: "起点"
                            font.pixelSize: 13
                            color: "#555"
                        }

                        Basic.TextField {
                            id: startInput
                            Layout.fillWidth: true
                            Layout.preferredHeight: 42
                            text: startAddress
                            placeholderText: "请输入起点"
                            font.pixelSize: 13
                            background: Rectangle {
                                color: "#ffffff"
                                border.color: "#ddd"
                                radius: 8
                            }
                            onTextChanged: {
                                startAddress = text;
                            }
                        }
                    }

                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 6

                        Text {
                            text: "终点"
                            font.pixelSize: 13
                            color: "#555"
                        }

                        Basic.TextField {
                            id: endInput
                            Layout.fillWidth: true
                            Layout.preferredHeight: 42
                            text: endAddress
                            placeholderText: "请输入终点"
                            font.pixelSize: 13
                            background: Rectangle {
                                color: "#ffffff"
                                border.color: "#ddd"
                                radius: 8
                            }
                            onTextChanged: {
                                endAddress = text;
                            }
                        }
                    }

                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 6

                        Text {
                            text: "规划策略"
                            font.pixelSize: 13
                            color: "#555"
                        }

                        Basic.ComboBox {
                            id: policySelect
                            Layout.fillWidth: true
                            Layout.preferredHeight: 42
                            model: ["速度优先", "费用优先", "距离优先", "避开高速", "躲避拥堵"]
                            currentIndex: currentPolicy
                            font.pixelSize: 13
                            background: Rectangle {
                                color: "#ffffff"
                                border.color: "#ddd"
                                radius: 8
                            }
                            onCurrentIndexChanged: {
                                currentPolicy = currentIndex;
                            }
                        }
                    }

                    Basic.Button {
                        id: searchBtn
                        Layout.fillWidth: true
                        Layout.preferredHeight: 48
                        text: isSearching ? "规划中..." : "开始规划"
                        font.pixelSize: 15
                        font.weight: Font.Medium
                        enabled: !isSearching
                        background: Rectangle {
                            color: isSearching ? "#a0c4f0" : "#2670e8"
                            radius: 10
                        }
                        contentItem: Text {
                            text: parent.text
                            font: parent.font
                            color: "#ffffff"
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }
                        onClicked: {
                            if (startAddress.trim() === "" || endAddress.trim() === "") return;
                            isSearching = true;
                            mapService.searchRoute(startAddress, endAddress, "driving", currentPolicy, currentCity);
                        }
                    }

                    Item { Layout.fillHeight: true }
                }
            }

            // 右侧：路线信息
            Rectangle {
                Layout.fillWidth: true
                Layout.fillHeight: true
                color: "#e9f2fe"
                radius: 16
                border.color: "#d0ddee"
                border.width: 1

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 16
                    spacing: 12

                    Text {
                        text: "🗺️ 路线信息"
                        font.pixelSize: 18
                        font.weight: Font.Bold
                        color: "#2670e8"
                    }

                    Text {
                        text: "背景地图已显示路线"
                        font.pixelSize: 14
                        color: "#666"
                    }

                    Text {
                        text: "起点: " + currentStartLoc
                        font.pixelSize: 12
                        color: "#555"
                        visible: currentStartLoc !== ""
                    }

                    Text {
                        text: "终点: " + currentEndLoc
                        font.pixelSize: 12
                        color: "#555"
                        visible: currentEndLoc !== ""
                    }

                    Item { Layout.fillHeight: true }
                }
            }
        }

        Behavior on y { NumberAnimation { duration: 300; easing.type: Easing.OutCubic } }
    }

    Connections {
        target: mapService
        function onRouteSearchSuccess(timeSeconds, distanceMeters, startLoc, endLoc, polyline) {
            console.log("[MapPanel] 路线规划成功");
            isSearching = false;
            
            currentStartLoc = startLoc;
            currentEndLoc = endLoc;
            
            updateMapRoute();
        }

        function onRouteSearchFailed(error) {
            console.log("[MapPanel] 路线规划失败:", error);
            isSearching = false;
        }
    }

    Component.onCompleted: {
        console.log("[MapPanel] 初始化完成 - 使用高德 JS API AMap.Driving 插件");
    }
}
