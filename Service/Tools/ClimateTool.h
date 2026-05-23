#ifndef CLIMATETOOL_H
#define CLIMATETOOL_H

#include "Service/AgentCore/AgentTool.h"

#include <QFuture>
#include <QPromise>

class ClimateTool : public AgentTool
{
    Q_OBJECT
public:
    explicit ClimateTool(QObject *parent = nullptr) : AgentTool(parent) {}

    QString name() const override { return "control_air_conditioner"; }
    QString description() const override
    {
        return QStringLiteral(
            "控制空调状态，包括温度设置、风扇开关和档位调节。"
            "当用户要求调节空调时，直接调用此工具。");
    }
    QJsonObject parametersSchema() const override
    {
        return QJsonObject{
            {"type", "object"},
            {"properties", QJsonObject{
                {"fan_active", QJsonObject{
                    {"type", "boolean"},
                    {"description", "风扇是否激活，true表示开启，false表示关闭"}
                }},
                {"left_temp", QJsonObject{
                    {"type", "integer"},
                    {"description", "左侧温度，范围16-30度"}
                }},
                {"right_temp", QJsonObject{
                    {"type", "integer"},
                    {"description", "右侧温度，范围16-30度"}
                }},
                {"fan_level", QJsonObject{
                    {"type", "integer"},
                    {"description", "风扇档位，0=自动，1-4=手动档位"}
                }}
            }},
            {"required", QJsonArray{}}
        };
    }

signals:
    void climateControlChanged(bool fanActive, int leftTemp,
                                 int rightTemp, int fanLevel);

public:
    QFuture<QString> execute(const QJsonObject& args) override
    {
        QPromise<QString> promise;
        promise.start();

        bool fanActive = args.value("fan_active").toBool(false);
        int leftTemp = qBound(16, args.value("left_temp").toInt(22), 30);
        int rightTemp = qBound(16, args.value("right_temp").toInt(22), 30);
        int fanLevel = qBound(0, args.value("fan_level").toInt(0), 4);

        emit climateControlChanged(fanActive, leftTemp, rightTemp, fanLevel);

        promise.addResult(QStringLiteral(
            "已成功调节空调：风扇%1，左侧温度%2°C，右侧温度%3°C，风扇档位%4")
            .arg(fanActive ? "开启" : "关闭")
            .arg(leftTemp)
            .arg(rightTemp)
            .arg(fanLevel));
        promise.finish();
        return promise.future();
    }
};

#endif // CLIMATETOOL_H
