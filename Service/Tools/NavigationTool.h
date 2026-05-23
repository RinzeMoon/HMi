#ifndef NAVIGATIONTOOL_H
#define NAVIGATIONTOOL_H

#include "Service/AgentCore/AgentTool.h"
#include "Service/MapService/MapService.h"

#include <QFuture>
#include <QPromise>
#include <QEventLoop>

class NavigationTool : public AgentTool
{
    Q_OBJECT
public:
    explicit NavigationTool(QObject *parent = nullptr) : AgentTool(parent) {}

    QString name() const override { return "search_navigation"; }
    QString description() const override
    {
        return QStringLiteral(
            "进行路线规划导航。当用户要求导航、去某个地方、找路时，直接调用此工具。"
            "start_address 是起点地址（可选，默认当前位置），"
            "end_address 是终点地址（必填），city 是城市名称（可选，默认北京）。");
    }
    QJsonObject parametersSchema() const override
    {
        return QJsonObject{
            {"type", "object"},
            {"properties", QJsonObject{
                {"start_address", QJsonObject{
                    {"type", "string"},
                    {"description", "起点地址，可选，不传则默认当前位置"}
                }},
                {"end_address", QJsonObject{
                    {"type", "string"},
                    {"description", "终点地址，必填"}
                }},
                {"city", QJsonObject{
                    {"type", "string"},
                    {"description", "城市名称，可选，默认北京"}
                }}
            }},
            {"required", QJsonArray{"end_address"}}
        };
    }

signals:
    void navigationRequested(const QString& startAddress, const QString& endAddress,
                              const QString& city);

public:
    QFuture<QString> execute(const QJsonObject& args) override
    {
        QPromise<QString> promise;
        promise.start();

        QString startAddr = args["start_address"].toString();
        QString endAddr = args["end_address"].toString();
        QString city = args["city"].toString("北京");

        if (endAddr.isEmpty()) {
            promise.addResult("Error: end_address is required");
            promise.finish();
            return promise.future();
        }

        MapService& ms = MapService::instance();
        ms.searchRoute(startAddr, endAddr, "driving", 0, city);
        emit navigationRequested(startAddr, endAddr, city);

        QEventLoop loop;
        QMetaObject::Connection c1, c2;

        c1 = connect(&ms, &MapService::routeSearchSuccess,
            this, [&](int timeSec, int distMeters, const QString&, const QString&, const QString&) {
                disconnect(c1);
                disconnect(c2);
                int minutes = timeSec / 60;
                double km = distMeters / 1000.0;
                promise.addResult(QString(
                    "已成功发起导航：从%1到%2，全程%3公里，预计%4分钟")
                    .arg(startAddr.isEmpty() ? "当前位置" : startAddr)
                    .arg(endAddr)
                    .arg(km, 0, 'f', 1)
                    .arg(minutes));
                promise.finish();
                loop.quit();
            });

        c2 = connect(&ms, &MapService::routeSearchFailed,
            this, [&](const QString& err) {
                disconnect(c1);
                disconnect(c2);
                promise.addResult(QString("导航请求失败：%1").arg(err));
                promise.finish();
                loop.quit();
            });

        loop.exec();
        return promise.future();
    }
};

#endif // NAVIGATIONTOOL_H
