# CarHMI 架构

## 整体架构

```mermaid
graph TB
    subgraph UI["QML UI Layer"]
        Dashboard["仪表盘 Dashboard"]
        Map["地图导航 MapPanel"]
        Music["音乐面板 MusicPanel"]
        AI["AI 对话 AIPanel"]
        Car3D["3D 车模 Car3DPanel"]
        Video["视频通话 VideoCallPanel"]
        Bottom["底部控制 BottomControls"]
    end

    subgraph Agent["Agent Core"]
        AgentLoop["AgentLoop<br/>ReAct 循环引擎"]
        ToolRegistry["ToolRegistry<br/>插件注册表"]
        ToolExecutor["ToolExecutor<br/>插件执行器"]
        MemoryManager["MemoryManager<br/>分层记忆"]
    end

    subgraph LLM["LLM Service"]
        LLMService["LLMService<br/>SSE 流式推理"]
    end

    subgraph Tools["Tools (插件)"]
        TimeTool["TimeTool"]
        WeatherTool["WeatherTool"]
        MusicTool["MusicTool"]
        ClimateTool["ClimateTool"]
        NavTool["NavigationTool"]
    end

    subgraph Services["Backend Services"]
        DataBus["DataBus<br/>UDP 数据总线"]
        MusicSvc["MusicService<br/>网易云 API"]
        MapSvc["MapService<br/>高德地图 API"]
        WeatherSvc["WeatherService<br/>OpenWeatherMap"]
        LocationSvc["LocationService<br/>GPS 定位"]
        AudioSvc["AudioService<br/>Whisper 语音识别"]
        VideoSvc["VideoCallService<br/>WebSocket + FFmpeg"]
        NetworkMgr["NetworkManager<br/>HTTP 封装"]
        ConfigReader["ConfigReader<br/>INI 配置"]
    end

    subgraph External["External"]
        DeepSeek["DeepSeek API"]
        SimTool["SimTool<br/>车辆物理模拟器"]
        CAN["CAN Adapter<br/>(预留)"]
        NeteaseAPI["NeteaseCloudMusicApi"]
        GaodeAPI["高德地图 API"]
        OWM["OpenWeatherMap"]
    end

    AI --> AgentLoop
    AgentLoop --> LLMService
    AgentLoop --> ToolRegistry
    ToolRegistry --> ToolExecutor
    ToolExecutor --> TimeTool & WeatherTool & MusicTool & ClimateTool & NavTool
    AgentLoop --> MemoryManager

    TimeTool -.-> ConfigReader
    WeatherTool --> WeatherSvc
    MusicTool --> MusicSvc
    ClimateTool --> Bottom
    NavTool --> MapSvc

    LLMService --> NetworkMgr
    NetworkMgr --> DeepSeek

    DataBus -.-> Dashboard & Bottom & Map & AgentLoop
    DataBus --> SimTool
    DataBus -.-> CAN

    MusicSvc --> NetworkMgr --> NeteaseAPI
    MapSvc --> NetworkMgr --> GaodeAPI
    WeatherSvc --> NetworkMgr --> OWM
    AudioSvc --> AgentLoop
    VideoSvc --> Video
```

## Agent ReAct 循环

```mermaid
sequenceDiagram
    participant U as User
    participant Q as AIPanel
    participant L as AgentLoop
    participant M as MemoryManager
    participant S as LLMService
    participant E as ToolExecutor
    participant T as AgentTool
    participant A as External API

    U->>Q: voice / text input
    Q->>L: run(userInput)

    Note over L: ReAct loop start
    L->>M: addMessage(user)
    L->>S: sendMessageWithContext(messages, tools)
    S->>A: POST DeepSeek (SSE)
    A-->>S: streaming deltas
    S-->>L: streamingDelta
    L-->>Q: streamingDelta -> bubble

    alt tool_calls
        S-->>L: toolCallRequested(name, args)
        L-->>Q: taskProgress running
        L->>E: executeBlocking(name, args)
        E->>T: execute(args)
        T->>A: HTTP request
        A-->>T: response
        T-->>E: result
        E-->>L: ToolResult
        L-->>Q: taskProgress done
        L->>M: addMessage(assistant + tool_result)
        Note over L: loop back to startRound
    else final answer
        S-->>L: chatFinished
        L-->>Q: thinkingFinished
    end

    Q-->>U: final response + TodoList
```

## 数据总线 (DataBus)

```mermaid
graph LR
    subgraph Sources["数据源"]
        SimTool["SimTool<br/>物理引擎<br/>F=P/v, 风阻, 滚阻<br/>20Hz UDP"]
        CAN["CAN Adapter<br/>(预留接口)"]
    end

    subgraph Bus["DataBus :12345"]
        Parser["JSON Parser"]
        Props["Q_PROPERTY<br/>speed/rpm/gear/soc/range<br/>leftTemp/rightTemp/fanLevel<br/>gpsLat/gpsLng/throttle/brake"]
    end

    subgraph Consumers["消费者"]
        Dashboard["仪表盘<br/>车速/转速指针"]
        Bottom["空调控制<br/>温度/风扇"]
        AgentLoop["AgentLoop<br/>setAdditionalContext()"]
        Map["地图<br/>GPS 定位"]
    end

    SimTool -->|UDP JSON| Parser
    CAN -.->|UDP JSON| Parser
    Parser --> Props
    Props -->|vehicleDataChanged| Dashboard & Bottom & AgentLoop & Map
```

## 工具插件体系

```mermaid
classDiagram
    class AgentTool {
        &lt;&lt;abstract&gt;&gt;
        +name() string
        +description() string
        +parametersSchema() json
        +execute(args) future
    }

    class ToolRegistry {
        +registerTool()
        +findTool()
        +toOpenAISchema()
    }

    class ToolExecutor {
        +executeBlocking()
    }

    class TimeTool {
        get_current_time
    }

    class WeatherTool {
        get_weather
    }

    class MusicTool {
        play_music
    }

    class ClimateTool {
        control_air_conditioner
    }

    class NavigationTool {
        search_navigation
    }

    AgentTool &lt;|-- TimeTool
    AgentTool &lt;|-- WeatherTool
    AgentTool &lt;|-- MusicTool
    AgentTool &lt;|-- ClimateTool
    AgentTool &lt;|-- NavigationTool
    ToolRegistry o-- AgentTool
    ToolExecutor --&gt; ToolRegistry
```

## SimTool 物理模型

```mermaid
flowchart TD
    Inputs["驾驶员输入<br/>油门 0-1, 刹车 0-1<br/>档位 P/R/N/D<br/>方向盘角度"]

    subgraph Physics["20Hz 物理 Tick"]
        Drive["驱动力<br/>F = P×throttle / v<br/>(低速钳制)"]
        Brake["制动力<br/>F = brake × m×g×1.2"]
        Drag["风阻<br/>F = 0.5×ρ×Cd×A×v²"]
        Roll["滚阻<br/>F = Crr×m×g"]
        Net["净力 = Drive - Brake - Drag - Roll"]
        Accel["加速度 = Net / m"]
        Speed["v += a×dt"]
        SOC["SOC -= P×dt / 3600 / kWh"]
    end

    Outputs["输出<br/>speed, rpm, soc<br/>range, gear, accel"]

    Inputs --> Physics
    Physics --> Outputs
    Outputs -->|UDP JSON| DataBus["DataBus :12345"]
```

## 目录结构

```mermaid
graph TB
    Root["CarHMI/"]
    Root --> Service["Service/"]
    Root --> QML["QML/"]
    Root --> Network["NetworkManager/"]
    Root --> Components["components/"]
    Root --> SimToolDir["../SimTool/"]
    Root --> Assets["3D Models & Fonts"]

    Service --> AgentCore["AgentCore/<br/>AgentLoop, ToolRegistry<br/>ToolExecutor, MemoryManager"]
    Service --> LLM["LLMService/"]
    Service --> Tools["Tools/<br/>5 个内置插件"]
    Service --> DataBusDir["DataBus/"]
    Service --> Music["MusicService/"]
    Service --> Map["MapService/"]
    Service --> Weather["WeatherService/"]
    Service --> Location["LocationService/"]
    Service --> Audio["AudioService/"]
    Service --> Video["VideoCallService/"]
    Service --> Config["ConfigReader/"]

    QML --> MAIN["MAIN/main.qml"]
    QML --> Panels["AIPanel, Dashboard<br/>MapPanel, MusicPanel<br/>CarPanel, Car3DPanel<br/>VideoCallPanel<br/>BottomControls"]
    QML --> Bars["Sidebar, StatusBar"]
    QML --> Fonts["fonts/"]
