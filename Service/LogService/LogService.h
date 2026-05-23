#pragma once

#include <QObject>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QMutex>

class LogService : public QObject
{
    Q_OBJECT
public:
    static LogService& instance();
    
    Q_INVOKABLE void logInfo(const QString& message);
    Q_INVOKABLE void logWarning(const QString& message);
    Q_INVOKABLE void logError(const QString& message);
    Q_INVOKABLE void logDebug(const QString& message);
    
private:
    LogService(QObject *parent = nullptr);
    ~LogService();
    
    void writeLog(const QString& level, const QString& message);
    
    QFile* m_logFile;
    QTextStream* m_textStream;
    QMutex m_mutex;
};