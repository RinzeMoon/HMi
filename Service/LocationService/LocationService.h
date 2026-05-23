//
// Created by admin on 2026/3/16.
//

#ifndef LOCATIONSERVICE_H
#define LOCATIONSERVICE_H

#include <QObject>
#include <QGeoPositionInfoSource>
#include <QTimer>

class LocationService : public QObject
{
    Q_OBJECT
public:
    static LocationService& instance();

    // 开始定位
    Q_INVOKABLE void startLocation();

    // 停止定位
    Q_INVOKABLE void stopLocation();

    // 获取当前位置
    Q_INVOKABLE QGeoPositionInfo currentPosition() const;

signals:
    // 位置更新
    void positionUpdated(double latitude, double longitude, const QString &address);

    // 定位错误
    void locationError(const QString &error);

private slots:
    void onPositionUpdated(const QGeoPositionInfo &position);
    void onPositionError(QGeoPositionInfoSource::Error error);

private:
    explicit LocationService(QObject *parent = nullptr);
    ~LocationService();

    QGeoPositionInfoSource *m_positionSource;
    QGeoPositionInfo m_currentPosition;
    QTimer* m_simTimer = nullptr;
    double m_simLat = 39.9087;
    double m_simLng = 116.3975;

    void startSimulation();
    void onSimulationTick();
};

#endif // LOCATIONSERVICE_H
