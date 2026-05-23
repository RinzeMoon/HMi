#ifndef AGENTLOOP_H
#define AGENTLOOP_H

#include <QObject>
#include <QJsonArray>
#include <QJsonObject>
#include <QAtomicInt>
#include <QTimer>
#include <memory>
#include <functional>

class LLMService;
class ToolRegistry;
class ToolExecutor;
class MemoryManager;

class AgentLoop : public QObject
{
    Q_OBJECT
public:
    explicit AgentLoop(QObject *parent = nullptr);
    ~AgentLoop();

    Q_INVOKABLE void run(const QString& userInput);
    Q_INVOKABLE void cancel();
    Q_INVOKABLE void clearMemory();
    Q_INVOKABLE void setAdditionalContext(const QString& ctx);

    void setMaxSteps(int n) { m_maxSteps = n; }
    int maxSteps() const { return m_maxSteps; }

    void setSystemPrompt(const QString& prompt);
    void setTools(const QJsonArray& tools) { m_toolsJson = tools; }
    void setRegistry(ToolRegistry* r) { m_registry = r; }
    void setExecutor(ToolExecutor* e) { m_executor = e; }

signals:
    void streamingDelta(const QString& delta);
    void thinkingStarted();
    void thinkingFinished();
    void stepUpdate(const QString& toolName, int step, int totalSteps);
    void taskProgress(const QString& taskName, const QString& status,
                       int step, int totalSteps);
    void finalResponse(const QString& content);
    void errorOccurred(const QString& error);

private slots:
    void onStreamingDelta(const QString& delta);
    void onToolCallRequested(const QString& name, const QJsonObject& args,
                              const QString& callId);
    void onChatFinished();

private:
    struct ToolCallRecord {
        QString id;
        QString name;
        QJsonObject args;
    };

    void startRound();
    bool executePendingToolCalls();
    QString buildContextString() const;

    LLMService* m_llm;
    ToolRegistry* m_registry;
    ToolExecutor* m_executor;
    MemoryManager* m_memory;
    QAtomicInt m_cancelled;

    QJsonArray m_toolsJson;
    QList<ToolCallRecord> m_pendingToolCalls;
    QString m_additionalContext;

    int m_maxSteps = 6;
    int m_currentStep = 0;
    int m_maxHistory = 24;  // keep at most 24 messages in working memory

    enum State { Idle, Thinking, Acting } m_state = Idle;
};

#endif // AGENTLOOP_H
