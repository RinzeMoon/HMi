#ifndef TIMESERVICE_H
#define TIMESERVICE_H

#include <QObject>
#include <QTimer>
#include <QDateTime>

class TimeService : public QObject
{
    Q_OBJECT

public:
    explicit TimeService(QObject *parent = nullptr);

    Q_INVOKABLE QString getCurrentTime();
    Q_INVOKABLE QString getCurrentDate();

signals:
    void timeUpdated(const QString &time, const QString &date);

private slots:
    void updateTime();

private:
    QTimer *m_timer;
    QString m_currentTime;
    QString m_currentDate;
};

#endif // TIMESERVICE_H
