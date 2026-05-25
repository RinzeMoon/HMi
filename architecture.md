# CarHMI 架构

## 整体架构

```mermaid
graph TD
    subgraph UI["QML UI"]
        AI["AIPanel"] --> AL["AgentLoop"]
        DB["Dashboard"] & MP["MapPanel"] & BC["BottomControls"] & VD["VideoCall"]
    end

    subgraph Core["Agent Core"]
        AL --> TR["ToolRegistry"] --> TE["ToolExecutor"]
        AL --> LS["LLMService"] & MM["MemoryManager"]
    end

    subgraph Plugins["Plugins"]
        TE --> T1["Time"] & T2["Weather"] & T3["Music"] & T4["Climate"] & T5["Nav"]
    end

    subgraph Svc["Services"]
        T2 --> WS["WeatherService"] --> OWM["OpenWeatherMap"]
        T3 --> MS["MusicService"] --> NCM["NeteaseAPI"]
        T5 --> NS["MapService"] --> GD["GaodeAPI"]
        LS --> NM["NetworkManager"] --> DS["DeepSeek API"]
        T1 --> CR["ConfigReader"]
        T4 --> BC
    end

    subgraph Bus["DataBus :12345"]
        DBus["DataBus"] -->|UDP| ST["SimTool"]
        DBus -.->|reserved| CAN["CAN Adapter"]
        DBus --> DB & BC & AL
    end

    ASvc["AudioService\nWhisper"] --> AL
    VSvc["VideoCallService\nFFmpeg+WS"] --> VD
```

## Agent ReAct 循环

```mermaid
sequenceDiagram
    participant U as 用户
    participant Q as AIPanel
    participant L as AgentLoop
    participant S as LLMService
    participant E as ToolExecutor
    participant A as 外部API

    U->>Q: 语音/文字输入
    Q->>L: run()
    activate L
    L->>S: 发起对话(messages+tools)
    S->>A: POST SSE 流式请求
    A-->>S: 流式增量返回
    S-->>L: 逐字推送
    L-->>Q: 气泡实时更新

    alt 有工具调用
        S-->>L: 返回 tool_calls
        L-->>Q: 显示进度(执行中)
        L->>E: 执行工具
        E->>A: HTTP 请求
        A-->>E: 响应结果
        E-->>L: 工具结果
        L-->>Q: 显示进度(已完成)
        Note right of L: 循环回到推理
    else 推理结束
        S-->>L: 无更多工具调用
        L-->>Q: 推理完成
    end
    deactivate L
    Q-->>U: 最终回复 + TodoList
```

## 数据总线

```mermaid
graph LR
    ST["SimTool\n物理引擎 20Hz"] -->|UDP JSON| DBus["DataBus :12345"]
    DBus -->|广播车辆数据| D["仪表盘"] & B["空调控制"] & AL["AgentLoop\n上下文注入"]
```

## 插件体系

```mermaid
classDiagram
    direction LR
    AgentTool : +name()
    AgentTool : +description()
    AgentTool : +schema()
    AgentTool : +execute()

    AgentTool &lt;|-- TimeTool : get_current_time
    AgentTool &lt;|-- WeatherTool : get_weather
    AgentTool &lt;|-- MusicTool : play_music
    AgentTool &lt;|-- ClimateTool : control_ac
    AgentTool &lt;|-- NavTool : search_nav

    ToolRegistry o-- AgentTool
    ToolExecutor --> ToolRegistry
```

## SimTool 物理

```mermaid
flowchart LR
    I["油门/刹车\n档位 P/R/N/D"] --> P["物理计算 20Hz"]
    P --> O["车速/转速\n续航/加速度"]
    O -->|UDP JSON| DBus["DataBus"]

    subgraph P
        direction LR
        F["驱动力-风阻-滚阻"] --> N["加速度"] --> V["速度累加"]
    end
```

## 目录结构

```mermaid
graph LR
    Root["CarHMI/"] --> Svc["Service/"] & QML["QML/"] & Net["NetworkManager/"] & Cmp["components/"] & Ast["3D/fonts"]
    Svc --> AC["AgentCore/"] & LS["LLMService/"] & TL["Tools/"] & DBus["DataBus/"] & etc["Music/Map/Weather/..."]
    QML --> Panels["AIPanel Dashboard\nMapPanel MusicPanel\nCar3DPanel VideoCall"]
```
