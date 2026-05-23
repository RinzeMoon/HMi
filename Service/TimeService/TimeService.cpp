#include "TimeService.h"
#include <QDateTime>
#include <QLocale>

TimeService::TimeService(QObject *parent)
    : QObject(parent)
{
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &TimeService::updateTime);
    
    // 每秒更新一次时间
    m_timer->start(1000);
    
    // 立即更新一次
    updateTime();
}

void TimeService::updateTime()
{
    QDateTime now = QDateTime::currentDateTime();
    m_currentTime = now.toString("HH:mm");
    m_currentDate = now.toString("yyyy-MM-dd");
    
    emit timeUpdated(m_currentTime, m_currentDate);
}

QString TimeService::getCurrentTime()
{
    return m_currentTime;
}

QString TimeService::getCurrentDate()
{
    return m_currentDate;
}
