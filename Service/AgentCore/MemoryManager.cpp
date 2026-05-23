#include "MemoryManager.h"

#include <QDebug>
#include <QDir>
#include <QJsonDocument>

MemoryManager::MemoryManager(QObject *parent) : QObject(parent)
{
    QString prefsPath = QDir::currentPath() + "/agent_prefs.ini";
    m_prefs = new QSettings(prefsPath, QSettings::IniFormat, this);
    qDebug() << "[MemoryManager] Preferences stored at:" << prefsPath;
}

MemoryManager::~MemoryManager() {}

MemoryManager& MemoryManager::instance()
{
    static MemoryManager instance;
    return instance;
}

// --- Working memory ---

void MemoryManager::addMessage(const QJsonObject& msg)
{
    m_conversation.append(msg);
    emit memoryUpdated();
}

QJsonArray MemoryManager::conversationMessages() const
{
    return m_conversation;
}

void MemoryManager::clearConversation()
{
    m_conversation = QJsonArray();
    m_lastSummary.clear();
    emit memoryUpdated();
}

int MemoryManager::conversationSize() const
{
    return m_conversation.size();
}

QJsonArray MemoryManager::buildMessages(const QString& systemPrompt) const
{
    QJsonArray msgs;
    QJsonObject sysMsg;
    sysMsg["role"] = "system";
    sysMsg["content"] = systemPrompt;
    msgs.append(sysMsg);

    for (int i = 0; i < m_conversation.size(); ++i) {
        msgs.append(m_conversation[i]);
    }
    return msgs;
}

void MemoryManager::trim(int maxMessages)
{
    while (m_conversation.size() > maxMessages) {
        m_conversation.removeFirst();
    }
}

// --- Short-term memory ---

void MemoryManager::saveConversationSummary(const QString& summary)
{
    m_lastSummary = summary;
}

QString MemoryManager::lastSummary() const
{
    return m_lastSummary;
}

// --- Long-term memory (QSettings-backed) ---

void MemoryManager::setUserPreference(const QString& key, const QString& value)
{
    m_prefs->beginGroup("Preferences");
    m_prefs->setValue(key, value);
    m_prefs->endGroup();
    m_prefs->sync();
}

QString MemoryManager::userPreference(const QString& key, const QString& defaultValue) const
{
    m_prefs->beginGroup("Preferences");
    QString v = m_prefs->value(key, defaultValue).toString();
    m_prefs->endGroup();
    return v;
}
