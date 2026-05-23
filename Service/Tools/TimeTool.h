#ifndef TIMETOOL_H
#define TIMETOOL_H

#include "Service/AgentCore/AgentTool.h"
#include <QFuture>
#include <QPromise>

class TimeTool : public AgentTool
{
    Q_OBJECT
public:
    explicit TimeTool(QObject *parent = nullptr) : AgentTool(parent) {}

    QString name() const override { return "get_current_time"; }
    QString description() const override
    {
        return QStringLiteral(
            "获取当前的时间和日期，不需要任何参数。"
            "当用户询问现在几点、今天几号等时间相关问题时使用此工具。");
    }
    QJsonObject parametersSchema() const override
    {
        return QJsonObject{
            {"type", "object"},
            {"properties", QJsonObject{}},
            {"required", QJsonArray{}}
        };
    }

    QFuture<QString> execute(const QJsonObject& /*args*/) override
    {
        QPromise<QString> promise;
        promise.start();
        promise.addResult(QDateTime::currentDateTime().toString(
            QStringLiteral("yyyy-MM-dd hh:mm:ss dddd")));
        promise.finish();
        return promise.future();
    }
};

#endif // TIMETOOL_H
