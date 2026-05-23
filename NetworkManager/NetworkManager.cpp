//
// Created by admin on 2026/3/12.
//

#include "NetworkManager.h"
#include <QNetworkRequest>

NetworkManager::NetworkManager(QObject *parent)
    : QObject(parent)
    , m_manager(new QNetworkAccessManager(this))
{
}

NetworkManager::~NetworkManager()
{
}

NetworkManager& NetworkManager::instance()
{
    static NetworkManager instance;
    return instance;
}

QNetworkReply* NetworkManager::get(const QUrl &url)
{
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::UserAgentHeader, "QtCarHMI/1.0");
    return m_manager->get(request);
}

QNetworkReply* NetworkManager::post(const QUrl &url, const QByteArray &data,
                                    const QString &contentType)
{
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::UserAgentHeader, "QtCarHMI/1.0");
    request.setHeader(QNetworkRequest::ContentTypeHeader, contentType);
    return m_manager->post(request, data);
}

QNetworkReply* NetworkManager::post(const QUrl &url, QHttpMultiPart *multiPart)
{
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::UserAgentHeader, "QtCarHMI/1.0");
    return m_manager->post(request, multiPart);
}

QNetworkReply* NetworkManager::sendRequest(const QNetworkRequest &request, const QByteArray &verb, const QByteArray &data)
{
    // 复制一份请求，以便统一设置 User-Agent（可选，如果调用者已设置则不覆盖）
    QNetworkRequest req = request;
    if (!req.hasRawHeader("User-Agent")) {
        req.setHeader(QNetworkRequest::UserAgentHeader, "QtCarHMI/1.0");
    }

    if (verb == "GET") {
        return m_manager->get(req);
    } else if (verb == "POST") {
        return m_manager->post(req, data);
    } else if (verb == "PUT") {
        return m_manager->put(req, data);
    } else if (verb == "DELETE") {
        return m_manager->deleteResource(req);
    } else {
        // 其他自定义方法，可以使用 sendCustomRequest
        return m_manager->sendCustomRequest(req, verb, data);
    }
}

QNetworkReply* NetworkManager::postWithHeaders(const QUrl &url, const QByteArray &data, 
                                                 const QMap<QByteArray, QByteArray> &headers,
                                                 const QString &contentType)
{
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::UserAgentHeader, "QtCarHMI/1.0");
    request.setHeader(QNetworkRequest::ContentTypeHeader, contentType);
    
    for (auto it = headers.begin(); it != headers.end(); ++it) {
        request.setRawHeader(it.key(), it.value());
    }
    
    return m_manager->post(request, data);
}