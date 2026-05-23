#include "VideoCallService.h"
#include <QDebug>
#include <QJsonArray>

VideoCallService& VideoCallService::instance()
{
    static VideoCallService instance;
    return instance;
}

VideoCallService::VideoCallService(QObject* parent)
    : QObject(parent)
    , m_webSocket(new QWebSocket(QString(), QWebSocketProtocol::VersionLatest, this))
    , m_networkManager(new QNetworkAccessManager(this))
    , m_callTimer(new QTimer(this))
    , m_reconnectTimer(new QTimer(this))
    , m_isConnected(false)
    , m_isInCall(false)
    , m_isMuted(false)
    , m_cameraEnabled(true)
    , m_callStatus("未连接")
    , m_callDuration(0)
    , m_reconnectAttempts(0)
{
    connect(m_webSocket, &QWebSocket::connected, this, &VideoCallService::onWebSocketConnected);
    connect(m_webSocket, &QWebSocket::disconnected, this, &VideoCallService::onWebSocketDisconnected);
    connect(m_webSocket, &QWebSocket::errorOccurred, this, &VideoCallService::onWebSocketError);
    connect(m_webSocket, &QWebSocket::textMessageReceived, this, &VideoCallService::onWebSocketTextMessageReceived);

    connect(m_callTimer, &QTimer::timeout, this, &VideoCallService::updateCallDuration);
    m_callTimer->setInterval(1000);

    connect(m_reconnectTimer, &QTimer::timeout, [this]() {
        if (m_reconnectAttempts < 5) {
            qDebug() << "[VideoCallService] 尝试重连... 次数:" << m_reconnectAttempts;
            startConnection();
            m_reconnectAttempts++;
        } else {
            m_reconnectTimer->stop();
            setErrorMessage("连接失败，请检查网络设置");
        }
    });
    m_reconnectTimer->setInterval(3000);
}

VideoCallService::~VideoCallService()
{
    if (m_webSocket) {
        m_webSocket->close();
    }
}

bool VideoCallService::isConnected() const
{
    return m_isConnected;
}

bool VideoCallService::isInCall() const
{
    return m_isInCall;
}

bool VideoCallService::isMuted() const
{
    return m_isMuted;
}

bool VideoCallService::cameraEnabled() const
{
    return m_cameraEnabled;
}

QString VideoCallService::callStatus() const
{
    return m_callStatus;
}

QString VideoCallService::errorMessage() const
{
    return m_errorMessage;
}

int VideoCallService::callDuration() const
{
    return m_callDuration;
}

void VideoCallService::startConnection()
{
    setCallStatus("正在连接...");
    m_errorMessage.clear();
    emit errorMessageChanged();
    
    QUrl url("ws://localhost:3000");
    m_webSocket->open(url);
    qDebug() << "[VideoCallService] 开始连接到" << url;
}

void VideoCallService::stopConnection()
{
    m_reconnectTimer->stop();
    m_reconnectAttempts = 0;
    
    if (m_isInCall) {
        endCall();
    }
    
    if (m_webSocket) {
        m_webSocket->close();
    }
    
    setCallStatus("已断开");
}

void VideoCallService::startCall()
{
    if (!m_isConnected) {
        setErrorMessage("请先连接服务器");
        return;
    }
    
    setCallStatus("正在发起通话...");
    
    QJsonObject message;
    message["type"] = "stream_start";
    sendWebSocketMessage(message);
    
    m_isInCall = true;
    m_callDuration = 0;
    m_callTimer->start();
    
    emit isInCallChanged();
    emit callDurationChanged();
    
    setCallStatus("通话中");
    
    QNetworkRequest request(QUrl("http://localhost:3000/stream-h264.ts"));
    request.setRawHeader("Cache-Control", "no-cache");
    request.setRawHeader("Connection", "keep-alive");
    
    QNetworkReply* reply = m_networkManager->get(request);
    connect(reply, &QNetworkReply::readyRead, [reply, this]() {
        QByteArray data = reply->readAll();
        if (!data.isEmpty()) {
            emit videoFrameReceived(data);
        }
    });
    connect(reply, &QNetworkReply::finished, reply, &QNetworkReply::deleteLater);
}

void VideoCallService::endCall()
{
    if (!m_isInCall) {
        return;
    }
    
    m_callTimer->stop();
    m_isInCall = false;
    
    emit isInCallChanged();
    
    setCallStatus("通话已结束");
}

void VideoCallService::toggleMute()
{
    m_isMuted = !m_isMuted;
    emit isMutedChanged();
    qDebug() << "[VideoCallService] 静音:" << m_isMuted;
}

void VideoCallService::toggleCamera()
{
    m_cameraEnabled = !m_cameraEnabled;
    emit cameraEnabledChanged();
    qDebug() << "[VideoCallService] 摄像头:" << m_cameraEnabled;
}

void VideoCallService::switchCamera()
{
    qDebug() << "[VideoCallService] 切换摄像头";
}

void VideoCallService::onWebSocketConnected()
{
    qDebug() << "[VideoCallService] WebSocket连接成功";
    
    m_isConnected = true;
    m_reconnectAttempts = 0;
    m_reconnectTimer->stop();
    
    emit isConnectedChanged();
    setCallStatus("已连接");
    
    QJsonObject message;
    message["type"] = "join";
    sendWebSocketMessage(message);
}

void VideoCallService::onWebSocketDisconnected()
{
    qDebug() << "[VideoCallService] WebSocket已断开";
    
    m_isConnected = false;
    
    if (m_isInCall) {
        endCall();
    }
    
    emit isConnectedChanged();
    setCallStatus("连接断开");
    
    m_reconnectTimer->start();
}

void VideoCallService::onWebSocketError()
{
    qDebug() << "[VideoCallService] WebSocket错误:" << m_webSocket->errorString();
    setErrorMessage("连接错误: " + m_webSocket->errorString());
}

void VideoCallService::onWebSocketTextMessageReceived(const QString& message)
{
    qDebug() << "[VideoCallService] 收到消息:" << message;
    
    QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8());
    if (!doc.isObject()) {
        return;
    }
    
    QJsonObject obj = doc.object();
    QString type = obj["type"].toString();
    
    if (type == "joined") {
        m_clientId = obj["clientId"].toString();
        qDebug() << "[VideoCallService] 已加入，客户端ID:" << m_clientId;
        setCallStatus("已就绪");
    }
}

void VideoCallService::onNetworkReplyFinished(QNetworkReply* reply)
{
    reply->deleteLater();
}

void VideoCallService::updateCallDuration()
{
    m_callDuration++;
    emit callDurationChanged();
}

void VideoCallService::sendWebSocketMessage(const QJsonObject& message)
{
    if (m_webSocket && m_webSocket->isValid()) {
        QJsonDocument doc(message);
        m_webSocket->sendTextMessage(doc.toJson(QJsonDocument::Compact));
    }
}

void VideoCallService::setCallStatus(const QString& status)
{
    if (m_callStatus != status) {
        m_callStatus = status;
        emit callStatusChanged();
    }
}

void VideoCallService::setErrorMessage(const QString& message)
{
    m_errorMessage = message;
    emit errorMessageChanged();
}
