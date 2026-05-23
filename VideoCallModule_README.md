# 音视频通话功能模块使用说明

## 概述

本模块为CarHMI项目提供了完整的音视频通话功能，实现了与api-interfaces.txt中定义的API规范的集成。

## 模块结构

### 文件组成

1. **Service/VideoCallService/**
   - `VideoCallService.h` - 音视频通话服务头文件
   - `VideoCallService.cpp` - 音视频通话服务实现文件

2. **QML/VideoCallPanel/**
   - `VideoCallWindow.qml` - 音视频通话窗口界面

3. **主界面集成**
   - 在main.qml中添加了VideoCallWindow组件
   - 添加了浮动按钮用于打开音视频通话窗口

## 功能特性

### 界面功能
- 独立的QML窗口组件
- 视频显示区域（远程视频和本地预览）
- 通话控制按钮：
  - 连接/断开服务器
  - 开始/结束通话
  - 静音/取消静音
  - 切换摄像头
  - 开关摄像头

### 数据处理功能
- WebSocket连接管理（ws://localhost:3000）
- HTTP视频流接收（http://localhost:3000/stream-h264.ts）
- 通话状态管理（连接中、已连接、通话中、已断开等）
- 自动重连机制（最多5次，间隔3秒）
- 通话时长计时

### API规范对接
- 完全遵循api-interfaces.txt中定义的接口规范
- 支持WebSocket消息格式（JSON）
- 实现了join、stream_start等消息类型
- 支持joined响应消息

## 使用方法

### 1. 启动服务器
首先需要启动Node.js服务器：
```bash
cd <服务器项目目录>
npm start
```

### 2. 启动CarHMI应用
运行构建好的CarHMI.exe

### 3. 打开音视频通话窗口
- 点击主界面左上角的蓝色圆形按钮（📹图标）
- 或者在代码中直接调用：
  ```qml
  videoCallWindow.visible = true
  videoCallWindow.show()
  ```

### 4. 建立连接
- 点击"连接"按钮建立与WebSocket服务器的连接
- 等待状态变为"已就绪"

### 5. 发起通话
- 点击绿色电话按钮（📞）开始通话
- 系统将自动连接到HTTP视频流

### 6. 通话控制
- **静音**：点击🔊/🔇按钮切换静音状态
- **摄像头**：点击📷按钮开关摄像头
- **切换摄像头**：点击🔄按钮切换前后摄像头
- **结束通话**：再次点击电话按钮结束通话

### 7. 断开连接
点击✓按钮断开与服务器的连接

## 技术实现

### C++后端（VideoCallService）
- 单例模式设计
- 继承自QObject，支持QML属性绑定
- 使用QWebSocket进行WebSocket通信
- 使用QNetworkAccessManager接收HTTP视频流
- 信号槽机制与QML界面交互

### QML界面（VideoCallWindow）
- 响应式设计，适配不同屏幕尺寸
- 使用Qt Quick Controls 2
- 深色主题设计，与CarHMI整体风格一致
- 实时状态显示和错误提示

### CMakeLists.txt配置
- 添加了Qt6::WebSockets依赖
- 包含了VideoCallService源文件

### main.cpp集成
- 注册videoCallService到QML上下文
- 导入VideoCallPanel模块

## 通话状态说明

| 状态 | 说明 |
|------|------|
| 未连接 | 初始状态，未连接服务器 |
| 正在连接... | 正在尝试连接WebSocket服务器 |
| 已连接 | WebSocket连接成功 |
| 已就绪 | 收到服务器joined响应，可以发起通话 |
| 正在发起通话... | 正在准备通话 |
| 通话中 | 通话进行中 |
| 通话已结束 | 通话已结束 |
| 连接断开 | 与服务器连接断开 |

## 注意事项

1. **服务器依赖**：必须先启动Node.js服务器才能使用本模块
2. **网络配置**：确保localhost:3000端口可访问
3. **FFmpeg**：项目已配置FFmpeg用于视频处理
4. **浏览器端**：需要在浏览器中打开http://localhost:3000并允许摄像头权限

## 扩展建议

1. **视频解码**：集成FFmpeg进行H.264视频解码和渲染
2. **音频处理**：添加音频采集和播放功能
3. **录制功能**：支持通话录制和回放
4. **多方通话**：支持多人音视频会议
5. **屏幕共享**：添加屏幕共享功能
6. **美颜效果**：添加视频美颜滤镜

## 文件修改记录

- 创建了VideoCallService.h和VideoCallService.cpp
- 创建了VideoCallWindow.qml
- 更新了CMakeLists.txt添加Qt WebSockets依赖和新源文件
- 更新了main.cpp注册新服务
- 更新了qml.qrc添加新的QML资源
- 更新了main.qml添加通话窗口和入口按钮

## 故障排除

### 连接失败
- 确认Node.js服务器是否正在运行
- 检查localhost:3000端口是否被占用
- 查看应用控制台日志获取详细错误信息

### 视频无法显示
- 确认浏览器端是否已允许摄像头权限
- 检查HTTP流地址是否正确
- 验证FFmpeg库是否正确配置

### 编译错误
- 确认Qt6 WebSockets模块已安装
- 检查CMakeLists.txt配置是否正确
- 清理build目录后重新构建
