//
// Created by admin on 2026/3/12.
//

#ifndef CARHMI_MAPSERVICE_H
#define CARHMI_MAPSERVICE_H

#include <QObject>
#include <QString>
#include <QNetworkAccessManager>
#include <QNetworkReply>

class MapService : public QObject
{
    Q_OBJECT
public:
    static MapService& instance();
    ~MapService();

    Q_INVOKABLE void searchRoute(const QString& start, const QString& end, 
                                  const QString& mode = "driving", int policy = 0, 
                                  const QString& city = "北京");
    Q_INVOKABLE QString getApiKey() const;
    Q_INVOKABLE QString getJsApiKey() const;
    Q_INVOKABLE QString getSecurityKey() const;

signals:
    void routeSearchSuccess(int timeSeconds, int distanceMeters, const QString& startLocation, const QString& endLocation, const QString& routePolyline);
    void routeSearchFailed(const QString& error);
    void routeSearchWithAllPaths(const QVariantList& paths);

private:
    explicit MapService(QObject *parent = nullptr);
    MapService(const MapService&) = delete;
    MapService& operator=(const MapService&) = delete;

    void geocodeStartAddress(const QString& address, const QString& city);
    void geocodeEndAddress(const QString& address, const QString& city);
    void searchRouteWithLocations(const QString& startLoc, const QString& endLoc, 
                                   const QString& mode, int policy);
    QString getErrorInfo(const QString& errCode);

    QNetworkAccessManager* networkManager;
    
    QString pendingEndAddress;
    QString pendingMode;
    int pendingPolicy;
    QString pendingCity;
    QString startLocation;
    QString endLocation;
};

#endif //CARHMI_MAPSERVICE_H
