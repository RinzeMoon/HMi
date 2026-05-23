#ifndef TOOLEXECUTOR_H
#define TOOLEXECUTOR_H

#include <QObject>
#include <QFuture>
#include <QJsonObject>
#include "ToolRegistry.h"

class ToolExecutor : public QObject
{
    Q_OBJECT
public:
    explicit ToolExecutor(ToolRegistry* registry, QObject *parent = nullptr)
        : QObject(parent), m_registry(registry) {}

    struct ToolResult {
        QString toolName;
        QString content;
        bool success;
        QString error;
    };

    ToolResult executeBlocking(const QString& toolName, const QJsonObject& args)
    {
        ToolResult result;
        result.toolName = toolName;

        AgentTool* tool = m_registry->findTool(toolName);
        if (!tool) {
            result.success = false;
            result.error = QString("Unknown tool: %1").arg(toolName);
            result.content = result.error;
            return result;
        }

        QFuture<QString> future = tool->execute(args);
        future.waitForFinished();  // wait synchronously inside agent loop

        if (future.isValid()) {
            result.success = true;
            result.content = future.result();
        } else {
            result.success = false;
            result.error = QString("Tool '%1' execution failed").arg(toolName);
            result.content = result.error;
        }

        return result;
    }

private:
    ToolRegistry* m_registry;
};

#endif // TOOLEXECUTOR_H
