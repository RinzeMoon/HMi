#include "AgentLoop.h"
#include "LLMService.h"
#include "ToolRegistry.h"
#include "ToolExecutor.h"
#include "MemoryManager.h"

#include <QDebug>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QDateTime>

AgentLoop::AgentLoop(QObject *parent)
    : QObject(parent)
    , m_llm(&LLMService::instance())
    , m_registry(nullptr)
    , m_executor(nullptr)
    , m_memory(&MemoryManager::instance())
{
    connect(m_llm, &LLMService::streamingDelta,
            this, &AgentLoop::onStreamingDelta);
    connect(m_llm, &LLMService::toolCallRequested,
            this, &AgentLoop::onToolCallRequested);
    connect(m_llm, &LLMService::chatFinished,
            this, &AgentLoop::onChatFinished);
}

AgentLoop::~AgentLoop() {}

void AgentLoop::setSystemPrompt(const QString& prompt)
{
    m_llm->setSystemPrompt(prompt);
}

void AgentLoop::cancel()
{
    m_cancelled.storeRelaxed(1);
    m_llm->cancel();
    m_pendingToolCalls.clear();
    m_state = Idle;
}

void AgentLoop::clearMemory()
{
    m_memory->clearConversation();
}

void AgentLoop::setAdditionalContext(const QString& ctx)
{
    m_additionalContext = ctx;
}

QString AgentLoop::buildContextString() const
{
    QStringList parts;
    parts << QDateTime::currentDateTime().toString(
        QStringLiteral("现在是 yyyy年M月d日 hh:mm:ss dddd"));

    // Inject user preferences if any
    QString homeAddr = m_memory->userPreference("home_address");
    QString workAddr = m_memory->userPreference("work_address");
    QString musicTaste = m_memory->userPreference("music_taste");
    if (!homeAddr.isEmpty())
        parts << QStringLiteral("用户的家在：%1").arg(homeAddr);
    if (!workAddr.isEmpty())
        parts << QStringLiteral("用户公司地址：%1").arg(workAddr);
    if (!musicTaste.isEmpty())
        parts << QStringLiteral("用户音乐偏好：%1").arg(musicTaste);

    // QML-injected context (speed, battery, climate, etc.)
    if (!m_additionalContext.isEmpty())
        parts << m_additionalContext;

    return parts.join(QStringLiteral("。"));
}

void AgentLoop::run(const QString& userInput)
{
    m_cancelled.storeRelaxed(0);
    m_pendingToolCalls.clear();
    m_currentStep = 0;

    // Continuous memory: don't clear; just add the new user message
    m_memory->trim(m_maxHistory);

    QJsonObject userMsg;
    userMsg["role"] = "user";
    userMsg["content"] = userInput;
    m_memory->addMessage(userMsg);

    emit thinkingStarted();
    startRound();
}

void AgentLoop::startRound()
{
    if (m_cancelled.loadRelaxed()) return;

    if (m_currentStep >= m_maxSteps) {
        qDebug() << "[AgentLoop] Max steps reached";
        emit thinkingFinished();
        emit errorOccurred("Reached maximum reasoning steps");
        m_state = Idle;
        return;
    }

    m_currentStep++;
    m_state = Thinking;

    qDebug() << "[AgentLoop] Starting round" << m_currentStep;

    // Build full system prompt with dynamic context
    QString basePrompt = m_llm->systemPrompt();
    QString context = buildContextString();
    QString fullPrompt = basePrompt + QStringLiteral("\n\n[当前环境信息]\n") + context;

    QJsonArray messages = m_memory->buildMessages(fullPrompt);
    m_llm->sendMessageWithContext(messages, m_toolsJson);
}

void AgentLoop::onStreamingDelta(const QString& delta)
{
    emit streamingDelta(delta);
}

static QString taskLabel(const QString& toolName)
{
    if (toolName == "get_current_time")    return QStringLiteral("查询时间");
    if (toolName == "play_music")          return QStringLiteral("搜索音乐");
    if (toolName == "get_weather")         return QStringLiteral("查询天气");
    if (toolName == "control_air_conditioner") return QStringLiteral("调节空调");
    if (toolName == "search_navigation")   return QStringLiteral("规划导航");
    return toolName;
}

void AgentLoop::onToolCallRequested(const QString& name, const QJsonObject& args,
                                      const QString& callId)
{
    ToolCallRecord rec;
    rec.name = name;
    rec.args = args;
    rec.id = callId;
    m_pendingToolCalls.append(rec);

    emit stepUpdate(name, m_currentStep, m_maxSteps);
    emit taskProgress(taskLabel(name), QStringLiteral("running"),
                       m_currentStep, m_maxSteps);
}

void AgentLoop::onChatFinished()
{
    if (m_cancelled.loadRelaxed()) {
        m_state = Idle;
        return;
    }

    if (m_pendingToolCalls.isEmpty()) {
        qDebug() << "[AgentLoop] Final response reached after" << m_currentStep << "steps";
        m_state = Idle;
        emit thinkingFinished();
        return;
    }

    m_state = Acting;
    if (!executePendingToolCalls()) {
        m_state = Idle;
        emit thinkingFinished();
        emit errorOccurred("Tool execution failed");
        return;
    }

    m_pendingToolCalls.clear();

    if (m_cancelled.loadRelaxed()) {
        m_state = Idle;
        return;
    }

    startRound();
}

bool AgentLoop::executePendingToolCalls()
{
    if (!m_registry || !m_executor) {
        qWarning() << "[AgentLoop] ToolRegistry or ToolExecutor not set";
        return false;
    }

    QString reasoning = m_llm->currentReasoningContent();

    for (const auto& tc : m_pendingToolCalls) {
        qDebug() << "[AgentLoop] Executing tool:" << tc.name;

        QJsonObject assistantMsg;
        assistantMsg["role"] = "assistant";
        if (!reasoning.isEmpty()) {
            assistantMsg["reasoning_content"] = reasoning;
        }
        QJsonArray toolCallsArray;
        QJsonObject tcObj;
        tcObj["id"] = tc.id;
        tcObj["type"] = "function";
        QJsonObject funcObj;
        funcObj["name"] = tc.name;
        funcObj["arguments"] = QString::fromUtf8(
            QJsonDocument(tc.args).toJson(QJsonDocument::Compact));
        tcObj["function"] = funcObj;
        toolCallsArray.append(tcObj);
        assistantMsg["tool_calls"] = toolCallsArray;
        m_memory->addMessage(assistantMsg);

        auto result = m_executor->executeBlocking(tc.name, tc.args);

        emit taskProgress(taskLabel(tc.name),
                           result.success ? QStringLiteral("done") : QStringLiteral("error"),
                           m_currentStep, m_maxSteps);

        QJsonObject toolMsg;
        toolMsg["role"] = "tool";
        toolMsg["tool_call_id"] = tc.id;
        toolMsg["content"] = result.success ? result.content
                    : QString("Error: %1").arg(result.error);
        m_memory->addMessage(toolMsg);
    }

    return true;
}
