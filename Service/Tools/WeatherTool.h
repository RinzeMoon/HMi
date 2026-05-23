#ifndef WEATHERTOOL_H
#define WEATHERTOOL_H

#include "Service/AgentCore/AgentTool.h"
#include "Service/WeatherService/WeatherService.h"

#include <QFuture>
#include <QPromise>
#include <QEventLoop>

class WeatherTool : public AgentTool
{
    Q_OBJECT
public:
    explicit WeatherTool(QObject *parent = nullptr) : AgentTool(parent) {}

    QString name() const override { return "get_weather"; }
    QString description() const override
    {
        return QStringLiteral(
            "获取指定城市的天气信息。city 参数填入城市名称。"
            "当用户询问某个城市的天气时，直接调用此工具。");
    }
    QJsonObject parametersSchema() const override
    {
        return QJsonObject{
            {"type", "object"},
            {"properties", QJsonObject{
                {"city", QJsonObject{
                    {"type", "string"},
                    {"description", "要查询天气的城市名称，例如：北京、上海、广州、深圳等"}
                }}
            }},
            {"required", QJsonArray{"city"}}
        };
    }

    QFuture<QString> execute(const QJsonObject& args) override
    {
        QPromise<QString> promise;
        promise.start();

        QString city = args["city"].toString();
        if (city.isEmpty()) {
            promise.addResult("Error: city is required");
            promise.finish();
            return promise.future();
        }

        WeatherService& ws = WeatherService::instance();
        ws.getWeatherByCity(city);

        QEventLoop loop;
        QMetaObject::Connection c1, c2;

        c1 = connect(&ws, &WeatherService::weatherDataReceived,
            this, [&](const QString& loc, int temp, const QString& cond, const QString&) {
                disconnect(c1);
                disconnect(c2);
                promise.addResult(QString("%1的天气：%2，%3°C").arg(loc, cond).arg(temp));
                promise.finish();
                loop.quit();
            });

        c2 = connect(&ws, &WeatherService::weatherDataError,
            this, [&](const QString& err) {
                disconnect(c1);
                disconnect(c2);
                promise.addResult(QString("获取天气信息失败：%1").arg(err));
                promise.finish();
                loop.quit();
            });

        loop.exec();
        return promise.future();
    }
};

#endif // WEATHERTOOL_H
