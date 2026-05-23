#ifndef AGENTTOOL_H
#define AGENTTOOL_H

#include <QObject>
#include <QString>
#include <QJsonObject>
#include <QFuture>

class AgentTool : public QObject
{
    Q_OBJECT
public:
    explicit AgentTool(QObject *parent = nullptr) : QObject(parent) {}

    virtual QString name() const = 0;
    virtual QString description() const = 0;
    virtual QJsonObject parametersSchema() const = 0;
    virtual QFuture<QString> execute(const QJsonObject& args) = 0;
};

#endif // AGENTTOOL_H
