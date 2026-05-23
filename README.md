# CarHMI

基于 **Qt 6 / QML** 的 ReAct Agent 车载 HMI 原型，支持语音交互、音乐播放、路线导航、视频通话和仪表盘模拟。

## 架构

```
AgentLoop (ReAct) → ToolRegistry → ToolExecutor → Time/Music/Weather/Nav/Climate
       ↓
LLMService (SSE streaming, DeepSeek API)
       ↓
MemoryManager (跨轮对话 + 偏好持久化)
       ↓
DataBus (UDP:12345) ← SimTool (车辆物理模拟器)
```

## 构建

```bash
# 需要 Qt 6.5+, CMake 3.19+, FFmpeg, Visual Studio 2022
mkdir build && cd build
cmake .. -G "Visual Studio 17 2022"
cmake --build . --config Debug
```

## 运行

1. 复制 `config.template.ini` 为 `config.ini`，填入 API 密钥
2. 启动 `CarHMI.exe`
3. (可选) 启动 `SimTool.exe` 进行车辆数据模拟

## 依赖

- Qt 6.5 (Core, Gui, Qml, Quick, Network, Multimedia, WebEngine, Positioning, OpenGLWidgets)
- FFmpeg (avcodec, avformat, avutil, swscale)
- Assimp (3D 模型加载)
- Whisper.cpp (语音识别)
- DeepSeek API / 高德地图 API / NeteaseCloudMusicApi / OpenWeatherMap

## 项目结构

```
CarHMI/
├── Service/
│   ├── AgentCore/       # Agent 引擎 (AgentLoop, ToolRegistry, MemoryManager)
│   ├── LLMService/      # LLM 流式推理
│   ├── Tools/           # 内置插件 (Time, Music, Weather, Nav, Climate)
│   ├── DataBus/         # UDP 数据总线
│   ├── MusicService/    # 网易云音乐 API
│   ├── MapService/      # 高德地图 API
│   ├── WeatherService/  # OpenWeatherMap
│   ├── VideoCallService/# 视频通话
│   └── ...
├── QML/                 # HMI 面板 (仪表盘、地图、音乐、AI对话、视频通话)
├── NetworkManager/      # HTTP 封装
├── SimTool/             # (独立项目) 车辆物理模拟器
└── components/          # 通用 QML 组件库
```
