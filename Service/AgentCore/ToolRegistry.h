#ifndef TOOLREGISTRY_H
#define TOOLREGISTRY_H

#include <QObject>
#include <QJsonArray>
#include <QJsonObject>
#include <QStringList>
#include <memory>
#include <unordered_map>
#include "AgentTool.h"

class ToolRegistry : public QObject
{
    Q_OBJECT
public:
    explicit ToolRegistry(QObject *parent = nullptr) : QObject(parent) {}

    void registerTool(std::unique_ptr<AgentTool> tool)
    {
        const QString n = tool->name();
        m_tools[n] = std::move(tool);
    }

    AgentTool* findTool(const QString& name) const
    {
        auto it = m_tools.find(name);
        return it != m_tools.end() ? it->second.get() : nullptr;
    }

    QJsonArray toOpenAISchema() const
    {
        QJsonArray tools;
        for (const auto& pair : m_tools) {
            AgentTool* t = pair.second.get();
            QJsonObject toolObj;
            toolObj["type"] = "function";

            QJsonObject func;
            func["name"] = t->name();
            func["description"] = t->description();
            func["parameters"] = t->parametersSchema();
            toolObj["function"] = func;

            tools.append(toolObj);
        }
        return tools;
    }

    QStringList toolNames() const
    {
        QStringList names;
        for (const auto& pair : m_tools) {
            names.append(pair.first);
        }
        return names;
    }

private:
    std::unordered_map<QString, std::unique_ptr<AgentTool>> m_tools;
};

#endif // TOOLREGISTRY_H
