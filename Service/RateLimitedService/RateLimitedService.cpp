//
// Created by admin on 2026/3/12.
//

#include "RateLimitedService.h"

RateLimitedService::RateLimitedService(int maxRequestsPerSecond, QObject *parent)
    : QObject(parent)
    , m_maxRequestsPerSecond(maxRequestsPerSecond)
{
    int interval = 1000 / m_maxRequestsPerSecond; // 转换为毫秒
    m_timer.setInterval(interval);
    connect(&m_timer, &QTimer::timeout, this, &RateLimitedService::processQueue);
    m_timer.start();
}

void RateLimitedService::enqueueRequest(std::function<void()> task)
{
    m_queue.enqueue(task);
}

void RateLimitedService::processQueue()
{
    if (!m_queue.isEmpty()) {
        auto task = m_queue.dequeue();
        task();
    }
}