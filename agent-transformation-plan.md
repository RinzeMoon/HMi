# CarHMI AI Agent 改造方案

## 一、当前AI封装现状

项目是一个 **车载HMI系统**（Qt/C++ + QML），AI相关代码涉及3个层次：

| 层 | 文件 | 实际做了什么 |
|---|---|---|
| **网络传输** | `NetworkManager` | 通用HTTP封装，Bearer认证，单一POST请求 |
| **AI调用** | `DeepSeekService` | 封装DeepSeek Chat Completions API，硬编码5个function calling工具，维护一个`QList<QJsonObject>`对话历史 |
| **"场景引擎"** | `SceneEngine` | **基本是空的** — 用关键词匹配（`input.contains("空调")`），假的置信度阈值，没有真实NLP能力 |
| **前端编排** | `AIPanel.qml` | **Tool call路由逻辑全部写在QML的JavaScript里**（`handleToolCall`函数），一个280行的巨型switch-case |

### 数据流现状

```
用户输入 → AIPanel.qml → DeepSeekService::sendMessage()
                              ↓
                         HTTP POST → DeepSeek API
                              ↓
                       onReplyReceived()
                              ↓
                    ┌─ 有tool_calls? ─┐
                    ↓                  ↓
         emit toolCallRequested    emit messageReceived
                    ↓                  ↓
         AIPanel.qml JS层         直接显示回复
         路由到各Service
         (handleToolCall 100行switch)
                    ↓
         deepSeekService.submitToolResult()
                    ↓
              再次调API → 最终回复
```

---

## 二、核心问题

1. **不是Agent，是单轮LLM Wrapper** — 用户说一句 → LLM回一句（或带一个tool call）→ 结束。没有自主的多步推理循环。
2. **Tool执行逻辑在QML层** — `AIPanel.qml:78-124` 是tool路由的唯一实现，C++层只管收发HTTP。业务逻辑散落在UI脚本里，测试困难，且随着tool数量增长会越来越臃肿。
3. **SceneEngine是假实现** — 关键词匹配+硬编码mock结果，完全不经过LLM，本质上是占位代码。
4. **没有规划能力** — 无法将"帮我去朝阳大悦城，路上放点音乐，顺便看看那边天气"分解为多步执行。
5. **没有持久记忆** — 对话历史在内存中，重启即丢失。没有用户偏好记忆（常用目的地、温度偏好、音乐口味）。
6. **没有错误恢复** — tool调用失败没有retry/fallback机制，一个工具失败整个流程断裂。
7. **Tool是硬编码的** — 5个tool在`getTools()`里手动构造JSON，无法动态注册/卸载，新增一个工具要改3个地方（Service、QML、信号）。

---

## 三、改造目标：从Chatbot到真正的Agent

真正的Agent = **感知 → 规划 → 执行 → 反馈** 的闭环。在车载场景下，这个循环是：

```
用户输入 → Agent Core(推理+规划) → Tool执行器 → 结果反馈 → Agent再推理 → ... → 最终回复
```

### 架构对比

**改造前（3层）**：
```
QML UI ←→ DeepSeekService ←→ DeepSeek API
              ↑ 手动调用
        各业务Service (Time/Music/Weather...)
```

**改造后（4层）**：
```
┌──────────────────────────────────────────────┐
│  QML UI Layer  (AIPanel)                      │
│  只负责：展示消息、收集输入、显示中间状态       │
└──────────────────┬───────────────────────────┘
                   │ signals/slots
┌──────────────────▼───────────────────────────┐
│  Agent Core  (新增 C++ 层)                    │
│  ┌─────────────────────────────────────┐     │
│  │ AgentLoop (ReAct循环引擎)            │     │
│  │  Thought → Action → Observation → ..│     │
│  ├─────────────────────────────────────┤     │
│  │ TaskPlanner (任务规划/分解)           │     │
│  ├─────────────────────────────────────┤     │
│  │ ToolRegistry (动态工具注册表)         │     │
│  ├─────────────────────────────────────┤     │
│  │ MemoryManager (短期+长期记忆)         │     │
│  └─────────────────────────────────────┘     │
└──────┬──────────────────┬───────────────────┘
       │ LLM调用          │ Tool执行
┌──────▼──────────┐  ┌───▼────────────────────┐
│ LLMService      │  │ ToolExecutor (C++层)    │
│ (重构自          │  │  TimeTool               │
│  DeepSeekService)│  │  MusicTool              │
│                  │  │  WeatherTool            │
│  多模型支持       │  │  NavigationTool         │
│  流式输出         │  │  ClimateControlTool     │
│  Token管理        │  │  (可动态注册新tool)      │
└─────────────────┘  └────────────────────────┘
```

---

## 四、具体改造步骤

### 第1步：重构 DeepSeekService → LLMService（通用LLM层）

**当前问题**：DeepSeek专用，不支持流式输出，工具JSON硬编码在Service里。

**改动**：
- 抽象出`LLMProvider`接口，支持DeepSeek / OpenAI / 本地模型切换
- 增加 **SSE流式解析**（当前只有`finished`信号，逐字输出才是车载场景该有的体验）
- 将tool定义从Service中移出，改为由外部注入

```cpp
// 新接口（示意）
class LLMProvider : public QObject {
    Q_OBJECT
public:
    virtual void chat(const QString& systemPrompt,
                      const QJsonArray& messages,
                      const QJsonArray& tools) = 0;
signals:
    void streamingDelta(const QString& delta);       // 逐字流式
    void toolCallRequested(const QString& toolName,
                           const QJsonObject& args);
    void chatFinished();
    void errorOccurred(const QString& error);
};
```

### 第2步：新建 ToolRegistry + ToolExecutor（C++层工具系统）

**当前问题**：工具定义在`DeepSeekService::getTools()`里硬编码，执行逻辑在QML JavaScript里。

**改动**：
- 定义`AgentTool`基类，每个tool封装 name / description / JSON Schema / execute()
- `ToolRegistry`负责注册、查找、序列化所有工具为LLM需要的JSON格式
- `ToolExecutor`在C++层同步/异步执行tool，结果直接回传给Agent循环

```cpp
class AgentTool {
public:
    virtual QString name() const = 0;
    virtual QString description() const = 0;
    virtual QJsonObject parametersSchema() const = 0;
    virtual QFuture<QString> execute(const QJsonObject& args) = 0;
};

class ToolRegistry {
public:
    void registerTool(std::unique_ptr<AgentTool> tool);
    QJsonArray toOpenAISchema() const;        // 序列化为LLM API需要的格式
    AgentTool* findTool(const QString& name) const;
};
```

这样AIPanel.qml的`handleToolCall`那100行switch-case全部删除，新增工具只需写一个新类+一行注册。

### 第3步：新建 AgentLoop — ReAct循环引擎（**核心**）

**当前问题**：`sendMessage` → `onReplyReceived` → if tool_call → QML手动处理 → `submitToolResult` → 再调API。这是被动的、需要外部驱动的。

**改动**：实现标准的ReAct循环，让循环在C++内部自主运转：

```cpp
void AgentLoop::run(const QString& userInput) {
    memory->addUserMessage(userInput);
    emit thinkingStarted();

    while (stepsRemaining-- > 0) {
        // Thought: LLM推理下一步该做什么
        auto response = llm->chat(memory->getMessages(),
                                   registry->toOpenAISchema());

        if (response.isToolCall()) {
            emit stepUpdate(response.toolName());  // "正在查询天气..."
            // Action: 执行工具
            auto result = executor->execute(
                response.toolName(), response.toolArgs());
            // Observation: 将结果反馈给LLM
            memory->addToolResult(result);
            continue;  // 继续循环
        } else {
            emit finalResponse(response.content());
            break;
        }
    }
    emit thinkingFinished();
}
```

关键参数：
- **maxSteps** = 5~8（防止无限循环）
- **stepTimeout** = 30s（单步超时）
- QML需要展示中间状态："正在查询天气..."、"正在规划路线...（第2/3步）"

### 第4步：MemoryManager — 分层记忆

**当前问题**：只有`QList<QJsonObject> conversationHistory`，无结构、不持久。

**改动**：

| 记忆类型 | 存储内容 | 生命周期 | 实现 |
|---------|---------|---------|------|
| **工作记忆** | 当前对话的完整messages（含tool calls/results） | 单次会话 | 内存QList |
| **短期记忆** | 最近N轮对话摘要 | 本次驾驶周期 | SQLite本地DB |
| **长期记忆** | 用户偏好（常用目的地、温度偏好、音乐口味） | 跨驾驶周期 | SQLite + 可选向量化 |

车载场景特有记忆：
- "每天早上8点上班路上喜欢听什么歌"
- "冬天喜欢空调26度"
- "常去的目的地Top 5"
- "上次导航中断的位置"

### 第5步：TaskPlanner — 复杂任务分解

**当前问题**：用户说"帮我去朝阳大悦城，路上放点音乐，顺便看看那边天气怎么样" — 当前系统做不到。

**改动**：
- 对于多意图输入，先调一次LLM做**意图分解**，拆成子任务列表
- 每个子任务独立执行ReAct循环
- 最终汇总结果

```cpp
struct SubTask {
    QString description;     // "导航到朝阳大悦城"
    int priority;            // 1=最高（导航先做）
    QList<ToolCall> steps;   // 该子任务的tool调用序列
};
```

### 第6步：AIPanel.qml 瘦身

**当前**：280行，包含tool路由、markdown渲染、语音录音、UI动画。

**改造后**：约120行，只保留：
- 消息展示（markdown渲染）
- 输入框 + 发送按钮
- 语音按钮
- `agentThinking` 状态展示（"正在查询天气..."、"正在规划路线...第2/3步"）
- 通过`Connections`绑定AgentCore的信号

删除：整个`handleToolCall`函数 + 所有tool路由逻辑。

---

## 五、文件变更清单

| 操作 | 文件 | 说明 |
|------|------|------|
| 重构 | `Service/DeepSeekService/` → `Service/LLMService/` | 通用LLM层，支持流式+多模型 |
| 新增 | `Service/AgentCore/AgentLoop.h/.cpp` | ReAct循环引擎 |
| 新增 | `Service/AgentCore/ToolRegistry.h/.cpp` | 工具注册表 |
| 新增 | `Service/AgentCore/ToolExecutor.h/.cpp` | 工具执行器 |
| 新增 | `Service/AgentCore/AgentTool.h` | 工具基类 |
| 新增 | `Service/AgentCore/MemoryManager.h/.cpp` | 分层记忆管理 |
| 新增 | `Service/AgentCore/TaskPlanner.h/.cpp` | 任务规划器 |
| 新增 | `Service/Tools/TimeTool.h/.cpp` | 各工具独立文件 |
| 新增 | `Service/Tools/MusicTool.h/.cpp` | |
| 新增 | `Service/Tools/WeatherTool.h/.cpp` | |
| 新增 | `Service/Tools/NavigationTool.h/.cpp` | |
| 新增 | `Service/Tools/ClimateTool.h/.cpp` | |
| 删除/重写 | `Service/SceneEngine/` | 假实现删除，功能被AgentCore吸收 |
| 大改 | `QML/AIPanel/AIPanel.qml` | 删除handleToolCall，只保留UI |
| 微调 | `main.cpp` | 注册新Service到QML context |
| 微调 | `config.ini` | 增加agent相关配置（maxSteps, timeout等） |

---

## 六、实施优先级

| 优先级 | 模块 | 理由 |
|--------|------|------|
| **P0** | ToolRegistry + AgentTool基类 | 基础设施，风险最低，改完就能把QML里的JS switch删掉，立竿见影 |
| **P0** | LLMService流式输出 | 车载场景对延迟敏感，逐字输出是体验底线 |
| **P1** | AgentLoop (ReAct) | 核心价值所在，让系统真正能"自主多步推理" |
| **P2** | MemoryManager | 让Agent有"记忆"，体验质的提升 |
| **P3** | TaskPlanner | 锦上添花，处理多意图场景 |
| **P4** | 多模型支持 | 未来可切换模型或本地部署 |

建议按P0→P1→P2→P3顺序实施，每步都可独立验证。P0+P1做完就已经是一个真正的Agent了。

---

## 七、关键设计决策点

1. **Tool执行同步 vs 异步**：导航、天气查询等IO操作用`QFuture`异步，时间查询等纯计算用同步。AgentLoop统一通过`QFuture`等待，超时则触发fallback。

2. **System Prompt管理**：当前system prompt硬编码在`DeepSeekService.cpp`两处（sendMessage和submitToolResult各一份）。改造后统一由`MemoryManager`管理，支持从配置文件加载。

3. **错误处理策略**：tool执行失败时，AgentLoop应将该tool结果标记为error返回给LLM，让LLM决定是重试、换方案还是告知用户。不超过2次连续重试。

4. **取消机制**：用户如果在Agent思考/执行中途发了新消息，应能取消当前循环。AgentLoop需要`cancel()`方法，通过`QAtomicInt`标志位实现。
