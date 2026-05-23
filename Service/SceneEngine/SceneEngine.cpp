#include "SceneEngine.h"
#include <QDebug>

SceneEngine::SceneEngine(QObject *parent) : QObject(parent)
{
    qDebug() << "[SceneEngine] 构造完成";
}

SceneEngine::~SceneEngine()
{
}

SceneEngine& SceneEngine::instance()
{
    static SceneEngine instance;
    return instance;
}

void SceneEngine::processUserInput(const QString& input)
{
    qDebug() << "[SceneEngine] 处理用户输入:" << input;
    executeProcessingPipeline(input);
}

void SceneEngine::handleUserConfirmation(bool confirmed)
{
    qDebug() << "[SceneEngine] 处理用户确认:" << confirmed;
}

void SceneEngine::executeProcessingPipeline(const QString& input)
{
    qDebug() << "[SceneEngine] 执行处理管道，输入:" << input;
    
    QJsonObject mockResult;
    
    if (input.contains("调低") || input.contains("空调")) {
        mockResult["primary_scene"] = "temperature_hot";
        mockResult["confidence"] = 0.95;
        mockResult["explanation"] = "用户明确要求调低空调温度";
    } else if (input.contains("热") || input.contains("好像")) {
        mockResult["primary_scene"] = "temperature_hot";
        mockResult["confidence"] = 0.87;
        mockResult["explanation"] = "用户可能觉得热，但表述不够明确";
    } else {
        mockResult["primary_scene"] = "unknown";
        mockResult["confidence"] = 0.5;
        mockResult["explanation"] = "无法准确理解用户意图";
    }
    
    handleSceneMatchResult(mockResult);
}

void SceneEngine::handleSceneMatchResult(const QJsonObject& result)
{
    qDebug() << "[SceneEngine] 处理场景匹配结果:" << result;
    
    QString scene = result["primary_scene"].toString();
    double confidence = result["confidence"].toDouble();
    QString explanation = result["explanation"].toString();
    
    qDebug() << "[SceneEngine] 场景:" << scene << "置信度:" << confidence;
    
    if (confidence >= 0.9) {
        qDebug() << "[SceneEngine] 高置信度，自动执行";
        QList<QVariantMap> mockActions;
        executeActionsWithSafety(mockActions);
        emit aiResponseGenerated("已为您调低空调温度，当前温度22度");
    } else if (confidence >= 0.85) {
        qDebug() << "[SceneEngine] 请求用户确认";
        emit confirmationRequested("我理解您是想调低空调温度，对吗？");
    } else {
        qDebug() << "[SceneEngine] 请求澄清";
        emit clarificationRequested("我没太理解，您能再说清楚一点吗？");
    }
}

void SceneEngine::executeActionsWithSafety(const QList<QVariantMap>& actions)
{
    qDebug() << "[SceneEngine] 安全执行动作，共" << actions.size() << "个动作";
    emit actionExecuted("temp_adjust", true, "空调温度已调整");
}
