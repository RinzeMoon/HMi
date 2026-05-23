//
// Created by admin on 2026/3/12.
//

#ifndef RATELIMITEDSERVICE_H
#define RATELIMITEDSERVICE_H

#include <QObject>
#include <QQueue>
#include <QTimer>
#include <functional>

class RateLimitedService : public QObject
{
    Q_OBJECT
public:
    explicit RateLimitedService(int maxRequestsPerSecond, QObject *parent = nullptr);

protected:
    // 派生类调用此方法将任务加入队列，按速率执行
    void enqueueRequest(std::function<void()> task);

private slots:
    void processQueue();

private:
    QQueue<std::function<void()>> m_queue;
    QTimer m_timer;
    int m_maxRequestsPerSecond;
};

#endif // RATELIMITEDSERVICE_H