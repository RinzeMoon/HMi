#include "LogService.h"
#include <QDir>
#include <QCoreApplication>

LogService::LogService(QObject *parent) : QObject(parent)
{
    // 创建log目录
    QString logDir = QCoreApplication::applicationDirPath() + "/log";
    QDir dir(logDir);
    if (!dir.exists()) {
        dir.mkpath(logDir);
    }
    
    // 创建日志文件
    QString logFileName = logDir + "/" + QDateTime::currentDateTime().toString("yyyyMMdd") + ".log";
    m_logFile = new QFile(logFileName);
    if (m_logFile->open(QIODevice::Append | QIODevice::Text)) {
        m_textStream = new QTextStream(m_logFile);
        *m_textStream << "[" << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") << "] " << "Log service started" << endl;
    }
}

LogService::~LogService()
{
    if (m_textStream) {
        delete m_textStream;
    }
    if (m_logFile) {
        m_logFile->close();
        delete m_logFile;
    }
}

LogService& LogService::instance()
{
    static LogService instance;
    return instance;
}

void LogService::logInfo(const QString& message)
{
    writeLog("INFO", message);
}

void LogService::logWarning(const QString& message)
{
    writeLog("WARNING", message);
}

void LogService::logError(const QString& message)
{
    writeLog("ERROR", message);
}

void LogService::logDebug(const QString& message)
{
    writeLog("DEBUG", message);
}

void LogService::writeLog(const QString& level, const QString& message)
{
    QMutexLocker locker(&m_mutex);
    if (m_textStream) {
        *m_textStream << "[" << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") << "] " << level << ": " << message << endl;
        m_textStream->flush();
    }
}