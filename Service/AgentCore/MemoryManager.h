#ifndef MEMORYMANAGER_H
#define MEMORYMANAGER_H

#include <QObject>
#include <QJsonArray>
#include <QJsonObject>
#include <QString>
#include <QSettings>

class MemoryManager : public QObject
{
    Q_OBJECT
public:
    static MemoryManager& instance();
    ~MemoryManager();

    // Working memory — current conversation
    void addMessage(const QJsonObject& msg);
    QJsonArray conversationMessages() const;
    Q_INVOKABLE void clearConversation();
    int conversationSize() const;

    // Trim old messages, keeping system prompt + last N exchanges
    void trim(int maxMessages);

    QJsonArray buildMessages(const QString& systemPrompt) const;

    // Short-term memory — recent session context
    void saveConversationSummary(const QString& summary);
    QString lastSummary() const;

    // Long-term memory — user preferences (persisted via QSettings)
    Q_INVOKABLE void setUserPreference(const QString& key, const QString& value);
    Q_INVOKABLE QString userPreference(const QString& key,
                                        const QString& defaultValue = "") const;

signals:
    void memoryUpdated();

private:
    explicit MemoryManager(QObject *parent = nullptr);
    MemoryManager(const MemoryManager&) = delete;
    MemoryManager& operator=(const MemoryManager&) = delete;

    QJsonArray m_conversation;
    QSettings* m_prefs;
    QString m_lastSummary;
};

#endif // MEMORYMANAGER_H
