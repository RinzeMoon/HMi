#include "DataBus.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>
#include <QTimer>

DataBus::DataBus(QObject *parent) : QObject(parent)
{
    m_socket = new QUdpSocket(this);

    // Watchdog: mark disconnected if no packet for 2 seconds
    m_timeoutTimer = new QTimer(this);
    m_timeoutTimer->setInterval(2000);
    m_timeoutTimer->setSingleShot(true);
    connect(m_timeoutTimer, &QTimer::timeout, this, [this]() {
        if (m_connected) {
            m_connected = false;
            emit connectedChanged();
        }
    });
}

DataBus& DataBus::instance()
{
    static DataBus inst;
    return inst;
}

void DataBus::start()
{
    if (m_socket->state() == QAbstractSocket::BoundState) return;

    if (m_socket->bind(QHostAddress::LocalHost, 12345)) {
        qDebug() << "[DataBus] Listening on udp://127.0.0.1:12345";
        connect(m_socket, &QUdpSocket::readyRead, this, [this]() {
            while (m_socket->hasPendingDatagrams()) {
                QByteArray datagram;
                datagram.resize(m_socket->pendingDatagramSize());
                m_socket->readDatagram(datagram.data(), datagram.size());
                parseDatagram(datagram);
            }
        });
    } else {
        qWarning() << "[DataBus] Failed to bind:" << m_socket->errorString();
    }
}

void DataBus::stop()
{
    m_socket->close();
}

void DataBus::parseDatagram(const QByteArray& data)
{
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(data, &err);
    if (err.error != QJsonParseError::NoError) return;

    QJsonObject obj = doc.object();
    bool changed = false;

    auto apply = [&](const QString& key, auto& member) {
        if (obj.contains(key)) {
            auto val = obj[key];
            using T = std::decay_t<decltype(member)>;
            if constexpr (std::is_same_v<T, double>)
                member = val.toDouble();
            else if constexpr (std::is_same_v<T, int>)
                member = val.toInt();
            else if constexpr (std::is_same_v<T, bool>)
                member = val.toBool();
            changed = true;
        }
    };

    apply("speed",     m_speed);
    apply("rpm",       m_rpm);
    apply("gear",      m_gear);
    apply("soc",       m_soc);
    apply("range",     m_range);
    apply("left_temp",  m_leftTemp);
    apply("right_temp", m_rightTemp);
    apply("fan_level",  m_fanLevel);
    apply("fan_active", m_fanActive);
    apply("gps_lat",    m_gpsLat);
    apply("gps_lng",    m_gpsLng);
    apply("throttle",   m_throttle);
    apply("brake",      m_brake);
    apply("steering",   m_steering);

    if (changed) {
        if (!m_connected) {
            m_connected = true;
            emit connectedChanged();
        }
        m_timeoutTimer->start();  // reset watchdog
        emit vehicleDataChanged();
    }
}
