#ifndef LLMSERVICE_H
#define LLMSERVICE_H

#include <QObject>
#include <QString>
#include <QJsonArray>
#include <QJsonObject>
#include <QNetworkReply>
#include <QList>

class LLMService : public QObject
{
    Q_OBJECT
public:
    static LLMService& instance();
    ~LLMService();

    Q_INVOKABLE void sendMessage(const QString& message);
    Q_INVOKABLE void sendMessageWithContext(const QJsonArray& messages,
                                             const QJsonArray& tools);
    void cancel();

    void setSystemPrompt(const QString& prompt) { m_systemPrompt = prompt; }
    QString systemPrompt() const { return m_systemPrompt; }
    QString currentReasoningContent() const { return m_currentReasoning; }

signals:
    void streamingDelta(const QString& delta);
    void messageComplete(const QString& fullContent);
    void toolCallRequested(const QString& toolName, const QJsonObject& args,
                           const QString& toolCallId);
    void chatFinished();
    void errorOccurred(const QString& error);

private:
    explicit LLMService(QObject *parent = nullptr);
    LLMService(const LLMService&) = delete;
    LLMService& operator=(const LLMService&) = delete;

    void chatWithTools(const QJsonArray& messages, const QJsonArray& tools);
    void onReplyReceived(QNetworkReply* reply);
    void processStreamChunk(const QByteArray& chunk);
    void flushAccumulatedToolCalls();

    struct ToolCallAccum {
        QString id;
        QString name;
        QString arguments;  // accumulated across chunks
    };

    QList<QJsonObject> m_conversationHistory;
    QString m_systemPrompt;
    QByteArray m_streamBuffer;
    QByteArray m_responseBody;
    QString m_currentContent;
    QString m_currentReasoning;
    bool m_streamingActive = false;
    QMap<int, ToolCallAccum> m_accumulatedToolCalls;
};

#endif // LLMSERVICE_H
