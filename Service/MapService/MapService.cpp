//
// Created by admin on 2026/3/12.
//

#include "MapService.h"
#include "../ConfigReader/ConfigReader.h"
#include <QUrl>
#include <QUrlQuery>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>

MapService::MapService(QObject *parent) : QObject(parent)
{
    networkManager = new QNetworkAccessManager(this);
    pendingPolicy = 0;
    qDebug() << "[MapService] Initialized";
}

MapService::~MapService()
{
}

MapService& MapService::instance()
{
    static MapService instance;
    return instance;
}

QString MapService::getApiKey() const
{
    return ConfigReader::instance().getGaoDeApiKey();
}

QString MapService::getJsApiKey() const
{
    return ConfigReader::instance().getGaoDeJsApiKey();
}

QString MapService::getSecurityKey() const
{
    return ConfigReader::instance().getGaoDeSecurityKey();
}

QString MapService::getErrorInfo(const QString& errCode)
{
    static QMap<QString, QString> errorMap = {
        {"10001", "INVALID_USER_KEY - Key不正确或过期"},
        {"10002", "SERVICE_NOT_AVAILABLE - 没有权限使用相应的服务"},
        {"10003", "DAILY_QUERY_OVER_LIMIT - 访问已超出日访问量"},
        {"10004", "ACCESS_TOO_FREQUENT - 单位时间内访问过于频繁"},
        {"10005", "INVALID_USER_IP - IP白名单出错"},
        {"10006", "INVALID_USER_DOMAIN - 绑定域名无效"},
        {"10007", "INVALID_USER_SIGNATURE - 数字签名未通过验证"},
        {"10008", "INVALID_USER_SCODE - MD5安全码未通过验证"},
        {"10009", "USERKEY_PLAT_NOMATCH - 请求key与绑定平台不符"},
        {"10010", "IP_QUERY_OVER_LIMIT - IP访问超限"},
        {"10011", "NOT_SUPPORT_HTTPS - 服务不支持https请求"},
        {"10012", "INSUFFICIENT_PRIVILEGES - 权限不足"},
        {"10013", "USER_KEY_RECYCLED - Key被删除"},
        {"10014", "QPS_HAS_EXCEEDED_THE_LIMIT - QPS超限"},
        {"10015", "GATEWAY_TIMEOUT - 受单机QPS限流限制"},
        {"10016", "SERVER_IS_BUSY - 服务器负载过高"},
        {"10017", "RESOURCE_UNAVAILABLE - 所请求的资源不可用"},
        {"20000", "INVALID_PARAMS - 请求参数非法"},
        {"20001", "MISSING_REQUIRED_PARAMS - 缺少必填参数"},
        {"20002", "ILLEGAL_REQUEST - 请求协议非法"},
        {"20003", "UNKNOWN_ERROR - 其他未知错误"},
        {"20800", "OUT_OF_SERVICE - 规划点不在中国陆地范围内"}
    };
    return errorMap.value(errCode, "未知错误");
}

void MapService::searchRoute(const QString& start, const QString& end, 
                              const QString& mode, int policy, const QString& city)
{
    qDebug() << "[MapService] Searching route from" << start << "to" << end << "in city:" << city;
    
    pendingEndAddress = end;
    pendingMode = mode;
    pendingPolicy = policy;
    pendingCity = city;
    
    geocodeStartAddress(start, city);
}

void MapService::geocodeStartAddress(const QString& address, const QString& city)
{
    QString apiKey = getApiKey();
    QUrl url("https://restapi.amap.com/v3/geocode/geo");
    QUrlQuery query;
    query.addQueryItem("address", address);
    query.addQueryItem("city", city);
    query.addQueryItem("key", apiKey);
    url.setQuery(query);

    qDebug() << "[MapService] Geocoding start address:" << address << "in city:" << city;

    QNetworkRequest request(url);
    QNetworkReply* reply = networkManager->get(request);

    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        if (reply->error() != QNetworkReply::NoError) {
            QString error = "网络请求失败: " + reply->errorString();
            qDebug() << "[MapService]" << error;
            emit routeSearchFailed(error);
            reply->deleteLater();
            return;
        }

        QByteArray data = reply->readAll();
        qDebug() << "[MapService] Raw response (driving):" << data.left(800);
        QJsonDocument doc = QJsonDocument::fromJson(data);
        QJsonObject obj = doc.object();

        QString status = obj.value("status").toString();
        if (status != "1") {
            QString infocode = obj.value("infocode").toString();
            QString info = obj.value("info").toString();
            QString error = QString("起点地理编码失败: %1\n错误码: %2\n%3")
                .arg(info)
                .arg(infocode)
                .arg(getErrorInfo(infocode));
            qDebug() << "[MapService]" << error;
            emit routeSearchFailed(error);
            reply->deleteLater();
            return;
        }

        QJsonArray geocodes = obj.value("geocodes").toArray();
        if (geocodes.isEmpty()) {
            QString error = "未找到起点地址的坐标";
            qDebug() << "[MapService]" << error;
            emit routeSearchFailed(error);
            reply->deleteLater();
            return;
        }

        QJsonObject geocode = geocodes.first().toObject();
        startLocation = geocode.value("location").toString();
        qDebug() << "[MapService] Start geocode success, location:" << startLocation;
        
        reply->deleteLater();
        
        geocodeEndAddress(pendingEndAddress, pendingCity);
    });
}

void MapService::geocodeEndAddress(const QString& address, const QString& city)
{
    QString apiKey = getApiKey();
    QUrl url("https://restapi.amap.com/v3/geocode/geo");
    QUrlQuery query;
    query.addQueryItem("address", address);
    query.addQueryItem("city", city);
    query.addQueryItem("key", apiKey);
    url.setQuery(query);

    qDebug() << "[MapService] Geocoding end address:" << address << "in city:" << city;

    QNetworkRequest request(url);
    QNetworkReply* reply = networkManager->get(request);

    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        if (reply->error() != QNetworkReply::NoError) {
            QString error = "网络请求失败: " + reply->errorString();
            qDebug() << "[MapService]" << error;
            emit routeSearchFailed(error);
            reply->deleteLater();
            return;
        }

        QByteArray data = reply->readAll();
        qDebug() << "[MapService] Raw response (driving):" << data.left(800);
        QJsonDocument doc = QJsonDocument::fromJson(data);
        QJsonObject obj = doc.object();

        QString status = obj.value("status").toString();
        if (status != "1") {
            QString infocode = obj.value("infocode").toString();
            QString info = obj.value("info").toString();
            QString error = QString("终点地理编码失败: %1\n错误码: %2\n%3")
                .arg(info)
                .arg(infocode)
                .arg(getErrorInfo(infocode));
            qDebug() << "[MapService]" << error;
            emit routeSearchFailed(error);
            reply->deleteLater();
            return;
        }

        QJsonArray geocodes = obj.value("geocodes").toArray();
        if (geocodes.isEmpty()) {
            QString error = "未找到终点地址的坐标";
            qDebug() << "[MapService]" << error;
            emit routeSearchFailed(error);
            reply->deleteLater();
            return;
        }

        QJsonObject geocode = geocodes.first().toObject();
        endLocation = geocode.value("location").toString();
        qDebug() << "[MapService] End geocode success, location:" << endLocation;
        
        reply->deleteLater();
        
        searchRouteWithLocations(startLocation, endLocation, pendingMode, pendingPolicy);
    });
}

void MapService::searchRouteWithLocations(const QString& startLoc, const QString& endLoc,
                                            const QString& mode, int policy)
{
    QString apiKey = getApiKey();
    QString routeUrl;

    if (mode == "driving") {
        // strategy 32 = 结合实时路况和历史数据，支持返回多条备选路线
        int effectivePolicy = 32;
        routeUrl = QString("https://restapi.amap.com/v3/direction/driving?origin=%1&destination=%2&strategy=%3&key=%4&alternatives=10&show_fields=polyline,steps")
            .arg(startLoc)
            .arg(endLoc)
            .arg(effectivePolicy)
            .arg(apiKey);
        qDebug() << "[MapService] Driving URL:" << routeUrl;
    } else if (mode == "walking") {
        routeUrl = QString("https://restapi.amap.com/v3/direction/walking?origin=%1&destination=%2&key=%3")
            .arg(startLoc)
            .arg(endLoc)
            .arg(apiKey);
    } else if (mode == "transit") {
        routeUrl = QString("https://restapi.amap.com/v3/direction/transit/integrated?origin=%1&destination=%2&city=北京&strategy=%3&key=%4")
            .arg(startLoc)
            .arg(endLoc)
            .arg(policy)
            .arg(apiKey);
    } else if (mode == "riding" || mode == "electric") {
        routeUrl = QString("https://restapi.amap.com/v3/direction/riding?origin=%1&destination=%2&key=%3")
            .arg(startLoc)
            .arg(endLoc)
            .arg(apiKey);
    }

    qDebug() << "[MapService] Searching route, mode:" << mode;

    QUrl url(routeUrl);
    QNetworkRequest request(url);
    QNetworkReply* reply = networkManager->get(request);

    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        if (reply->error() != QNetworkReply::NoError) {
            QString error = "网络请求失败: " + reply->errorString();
            qDebug() << "[MapService]" << error;
            emit routeSearchFailed(error);
            reply->deleteLater();
            return;
        }

        QByteArray data = reply->readAll();
        qDebug() << "[MapService] Raw response (driving):" << data.left(800);
        QJsonDocument doc = QJsonDocument::fromJson(data);
        QJsonObject obj = doc.object();

        QString status = obj.value("status").toString();
        if (status != "1") {
            QString infocode = obj.value("infocode").toString();
            QString info = obj.value("info").toString();
            QString error = QString("路线规划失败: %1\n错误码: %2\n%3")
                .arg(info)
                .arg(infocode)
                .arg(getErrorInfo(infocode));
            if (infocode == "10009") {
                error += "\n\n💡 请确保使用的是「Web服务」类型的Key！";
            }
            qDebug() << "[MapService]" << error;
            emit routeSearchFailed(error);
            reply->deleteLater();
            return;
        }

        QJsonObject route = obj.value("route").toObject();
        QVariantList allPaths;
        int time = -1, distance = -1;
        QString polylineStr;

        QJsonArray paths = route.value("paths").toArray();
        qDebug() << "[MapService] Paths array size:" << paths.size();
        if (!paths.isEmpty()) {
            for (int i = 0; i < paths.size(); i++) {
                QJsonObject path = paths[i].toObject();
                int pathTime = path.value("duration").toString().toInt();
                int pathDistance = path.value("distance").toString().toInt();
                
                // 提取路线坐标
                QJsonArray steps = path.value("steps").toArray();
                QStringList allCoords;
                for (const QJsonValue& stepVal : steps) {
                    QJsonObject step = stepVal.toObject();
                    QString polyline = step.value("polyline").toString();
                    if (!polyline.isEmpty()) {
                        allCoords << polyline;
                    }
                }
                QString pathPolyline = allCoords.join(";");
                
                QVariantMap pathInfo;
                pathInfo["index"] = i;
                pathInfo["time"] = pathTime;
                pathInfo["distance"] = pathDistance;
                pathInfo["polyline"] = pathPolyline;
                allPaths.append(pathInfo);
                
                // 保存第一条路线用于兼容旧信号
                if (i == 0) {
                    time = pathTime;
                    distance = pathDistance;
                    polylineStr = pathPolyline;
                }
            }
        }

        if (time != -1 && distance != -1) {
            qDebug() << "[MapService] Route found -" << paths.size() << "paths";
            qDebug() << "[MapService] First path - time:" << time << "s, distance:" << distance << "m";
            emit routeSearchSuccess(time, distance, startLocation, endLocation, polylineStr);
            emit routeSearchWithAllPaths(allPaths);
        } else {
            QString error = "未找到有效路线";
            qDebug() << "[MapService]" << error;
            emit routeSearchFailed(error);
        }

        reply->deleteLater();
    });
}
