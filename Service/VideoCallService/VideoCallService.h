#ifndef VIDEOCALLSERVICE_H
#define VIDEOCALLSERVICE_H

#include <QObject>
#include <QTimer>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QWebSocket>
#include <QUrl>
#include <QJsonObject>
#include <QJsonDocument>

class VideoCallService : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool isConnected READ isConnected NOTIFY isConnectedChanged)
    Q_PROPERTY(bool isInCall READ isInCall NOTIFY isInCallChanged)
    Q_PROPERTY(bool isMuted READ isMuted NOTIFY isMutedChanged)
    Q_PROPERTY(bool cameraEnabled READ cameraEnabled NOTIFY cameraEnabledChanged)
    Q_PROPERTY(QString callStatus READ callStatus NOTIFY callStatusChanged)
    Q_PROPERTY(QString errorMessage READ errorMessage NOTIFY errorMessageChanged)
    Q_PROPERTY(int callDuration READ callDuration NOTIFY callDurationChanged)

public:
    static VideoCallService& instance();

    bool isConnected() const;
    bool isInCall() const;
    bool isMuted() const;
    bool cameraEnabled() const;
    QString callStatus() const;
    QString errorMessage() const;
    int callDuration() const;

    Q_INVOKABLE void startConnection();
    Q_INVOKABLE void stopConnection();
    Q_INVOKABLE void startCall();
    Q_INVOKABLE void endCall();
    Q_INVOKABLE void toggleMute();
    Q_INVOKABLE void toggleCamera();
    Q_INVOKABLE void switchCamera();

signals:
    void isConnectedChanged();
    void isInCallChanged();
    void isMutedChanged();
    void cameraEnabledChanged();
    void callStatusChanged();
    void errorMessageChanged();
    void callDurationChanged();
    void videoFrameReceived(const QByteArray& frameData);

private slots:
    void onWebSocketConnected();
    void onWebSocketDisconnected();
    void onWebSocketError();
    void onWebSocketTextMessageReceived(const QString& message);
    void onNetworkReplyFinished(QNetworkReply* reply);
    void updateCallDuration();

private:
    explicit VideoCallService(QObject* parent = nullptr);
    ~VideoCallService();

    void sendWebSocketMessage(const QJsonObject& message);
    void setCallStatus(const QString& status);
    void setErrorMessage(const QString& message);

    QWebSocket* m_webSocket;
    QNetworkAccessManager* m_networkManager;
    QTimer* m_callTimer;
    QTimer* m_reconnectTimer;

    bool m_isConnected;
    bool m_isInCall;
    bool m_isMuted;
    bool m_cameraEnabled;
    QString m_callStatus;
    QString m_errorMessage;
    QString m_clientId;
    int m_callDuration;
    int m_reconnectAttempts;
};

#endif
