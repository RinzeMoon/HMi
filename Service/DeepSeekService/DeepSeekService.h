#ifndef DEEPSEEKSERVICE_H
#define DEEPSEEKSERVICE_H

#include <QObject>
#include <QString>
#include <QNetworkReply>
#include <QJsonArray>
#include <QJsonObject>

class DeepSeekService : public QObject
{
    Q_OBJECT
public:
    static DeepSeekService& instance();
    ~DeepSeekService();

    Q_INVOKABLE void sendMessage(const QString& message);
    Q_INVOKABLE void submitToolResult(const QString& result);

signals:
    void messageReceived(const QString& role, const QString& content);
    void errorOccurred(const QString& error);
    void typingStarted();
    void typingFinished();
    void toolCallRequested(const QString& toolName, const QString& toolArgs);

private:
    explicit DeepSeekService(QObject *parent = nullptr);
    DeepSeekService(const DeepSeekService&) = delete;
    DeepSeekService& operator=(const DeepSeekService&) = delete;

    void onReplyReceived(QNetworkReply* reply);
    QJsonArray getTools();
    void sendMessageWithTools(const QJsonArray& messages);

    QList<QJsonObject> conversationHistory;
    QString pendingToolCallId;
};

#endif // DEEPSEEKSERVICE_H
