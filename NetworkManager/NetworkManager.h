//
// Created by admin on 2026/3/12.
//

#ifndef NETWORKMANAGER_H
#define NETWORKMANAGER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrl>
#include <QHttpMultiPart>

class NetworkManager : public QObject
{
    Q_OBJECT
public:
    static NetworkManager& instance();

    // 简单的 GET 请求（自动设置 User-Agent）
    QNetworkReply* get(const QUrl &url);

    // 简单的 POST 请求（自动设置 User-Agent 和 Content-Type）
    QNetworkReply* post(const QUrl &url, const QByteArray &data,
                        const QString &contentType = "application/x-www-form-urlencoded");
    
    // multipart/form-data POST 请求
    QNetworkReply* post(const QUrl &url, QHttpMultiPart *multiPart);

    // 通用请求方法，允许自定义 request 和 HTTP 动词（GET/POST 等）
    QNetworkReply* sendRequest(const QNetworkRequest &request, const QByteArray &verb = "GET", const QByteArray &data = QByteArray());
    
    // 带自定义 Header 的 POST 请求
    QNetworkReply* postWithHeaders(const QUrl &url, const QByteArray &data, 
                                     const QMap<QByteArray, QByteArray> &headers,
                                     const QString &contentType = "application/json");

private:
    explicit NetworkManager(QObject *parent = nullptr);
    ~NetworkManager();

    QNetworkAccessManager *m_manager;
};

#endif // NETWORKMANAGER_H