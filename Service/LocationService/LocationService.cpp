//
// Created by admin on 2026/3/16.
//

#include "LocationService.h"
#include "ConfigReader.h"
#include <QRandomGenerator>
#include <QDebug>

LocationService::LocationService(QObject *parent) : QObject(parent)
{
    m_positionSource = QGeoPositionInfoSource::createDefaultSource(this);
    if (m_positionSource) {
        connect(m_positionSource, SIGNAL(positionUpdated(QGeoPositionInfo)),
                this, SLOT(onPositionUpdated(QGeoPositionInfo)));
        connect(m_positionSource, SIGNAL(error(QGeoPositionInfoSource::Error)),
                this, SLOT(onPositionError(QGeoPositionInfoSource::Error)));
        m_positionSource->setUpdateInterval(10000);
    } else if (ConfigReader::instance().isSimulationEnabled()) {
        qDebug() << "[LocationService] No GPS hardware — simulation enabled via config";
        startSimulation();
    } else {
        qDebug() << "[LocationService] No GPS hardware — simulation disabled, waiting for DataBus";
    }
}

LocationService::~LocationService()
{
    if (m_positionSource) {
        delete m_positionSource;
    }
}

LocationService &LocationService::instance()
{
    static LocationService instance;
    return instance;
}

void LocationService::startSimulation()
{
    ConfigReader& cfg = ConfigReader::instance();
    m_simLat = cfg.simulationLat();
    m_simLng = cfg.simulationLng();

    m_simTimer = new QTimer(this);
    m_simTimer->setInterval(10000);
    connect(m_simTimer, &QTimer::timeout, this, &LocationService::onSimulationTick);
    m_simTimer->start();

    // Emit initial position immediately
    onSimulationTick();

    qDebug() << "[LocationService] Simulation started at" << m_simLat << m_simLng;
}

void LocationService::onSimulationTick()
{
    // Small random drift ~50-100m
    double driftLat = (QRandomGenerator::global()->bounded(200) - 100) / 100000.0;
    double driftLng = (QRandomGenerator::global()->bounded(200) - 100) / 100000.0;
    m_simLat += driftLat;
    m_simLng += driftLng;

    QString address = QString::number(m_simLat, 'f', 6) + ", "
                    + QString::number(m_simLng, 'f', 6);
    emit positionUpdated(m_simLat, m_simLng, address);
}

void LocationService::startLocation()
{
    if (m_positionSource) {
        m_positionSource->startUpdates();
    } else if (m_simTimer) {
        if (!m_simTimer->isActive()) {
            m_simTimer->start();
            onSimulationTick();
        }
    } else {
        emit locationError("No position source available");
    }
}

void LocationService::stopLocation()
{
    if (m_positionSource) {
        m_positionSource->stopUpdates();
    }
    if (m_simTimer) {
        m_simTimer->stop();
    }
}

QGeoPositionInfo LocationService::currentPosition() const
{
    return m_currentPosition;
}

void LocationService::onPositionUpdated(const QGeoPositionInfo &position)
{
    m_currentPosition = position;
    if (position.isValid()) {
        QGeoCoordinate coordinate = position.coordinate();
        double latitude = coordinate.latitude();
        double longitude = coordinate.longitude();
        QString address = QString::number(latitude, 'f', 6) + ", "
                        + QString::number(longitude, 'f', 6);
        emit positionUpdated(latitude, longitude, address);
    }
}

void LocationService::onPositionError(QGeoPositionInfoSource::Error error)
{
    QString errorString;
    switch (error) {
    case QGeoPositionInfoSource::AccessError:
        errorString = "Access error";
        break;
    case QGeoPositionInfoSource::ClosedError:
        errorString = "Closed error";
        break;
    default:
        errorString = "Unknown error";
        break;
    }
    emit locationError(errorString);
}
