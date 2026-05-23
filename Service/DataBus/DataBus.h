#ifndef DATABUS_H
#define DATABUS_H

#include <QObject>
#include <QUdpSocket>
#include <QJsonObject>

class DataBus : public QObject
{
    Q_OBJECT

    Q_PROPERTY(double speed READ speed NOTIFY vehicleDataChanged)
    Q_PROPERTY(int rpm READ rpm NOTIFY vehicleDataChanged)
    Q_PROPERTY(int gear READ gear NOTIFY vehicleDataChanged)
    Q_PROPERTY(double soc READ soc NOTIFY vehicleDataChanged)
    Q_PROPERTY(double range READ range NOTIFY vehicleDataChanged)
    Q_PROPERTY(double leftTemp READ leftTemp NOTIFY vehicleDataChanged)
    Q_PROPERTY(double rightTemp READ rightTemp NOTIFY vehicleDataChanged)
    Q_PROPERTY(int fanLevel READ fanLevel NOTIFY vehicleDataChanged)
    Q_PROPERTY(bool fanActive READ fanActive NOTIFY vehicleDataChanged)
    Q_PROPERTY(double gpsLat READ gpsLat NOTIFY vehicleDataChanged)
    Q_PROPERTY(double gpsLng READ gpsLng NOTIFY vehicleDataChanged)
    Q_PROPERTY(double throttle READ throttle NOTIFY vehicleDataChanged)
    Q_PROPERTY(double brake READ brake NOTIFY vehicleDataChanged)
    Q_PROPERTY(double steering READ steering NOTIFY vehicleDataChanged)
    Q_PROPERTY(bool connected READ connected NOTIFY connectedChanged)

public:
    static DataBus& instance();

    double speed() const { return m_speed; }
    int rpm() const { return m_rpm; }
    int gear() const { return m_gear; }
    double soc() const { return m_soc; }
    double range() const { return m_range; }
    double leftTemp() const { return m_leftTemp; }
    double rightTemp() const { return m_rightTemp; }
    int fanLevel() const { return m_fanLevel; }
    bool fanActive() const { return m_fanActive; }
    double gpsLat() const { return m_gpsLat; }
    double gpsLng() const { return m_gpsLng; }
    double throttle() const { return m_throttle; }
    double brake() const { return m_brake; }
    double steering() const { return m_steering; }
    bool connected() const { return m_connected; }

    Q_INVOKABLE void start();
    Q_INVOKABLE void stop();

signals:
    void vehicleDataChanged();
    void connectedChanged();

private:
    explicit DataBus(QObject *parent = nullptr);
    void parseDatagram(const QByteArray& data);

    QUdpSocket* m_socket = nullptr;
    QTimer* m_timeoutTimer = nullptr;

    double m_speed = 0;
    int m_rpm = 0;
    int m_gear = 0;  // 0=P 1=R 2=N 3=D 4..=manual
    double m_soc = 80;
    double m_range = 350;
    double m_leftTemp = 22;
    double m_rightTemp = 22;
    int m_fanLevel = 0;
    bool m_fanActive = false;
    double m_gpsLat = 39.9087;
    double m_gpsLng = 116.3975;
    double m_throttle = 0;
    double m_brake = 0;
    double m_steering = 0;
    bool m_connected = false;
};

#endif // DATABUS_H
