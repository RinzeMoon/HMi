#ifndef SCENEENGINE_H
#define SCENEENGINE_H

#include <QObject>
#include <QString>
#include <QVariantMap>
#include <QList>
#include <QJsonObject>

class SceneEngine : public QObject
{
    Q_OBJECT
public:
    static SceneEngine& instance();
    ~SceneEngine();

    Q_INVOKABLE void processUserInput(const QString& input);
    Q_INVOKABLE void handleUserConfirmation(bool confirmed);

signals:
    void actionExecuted(const QString& actionId, bool success, const QString& message);
    void confirmationRequested(const QString& message);
    void clarificationRequested(const QString& message);
    void aiResponseGenerated(const QString& response);

private:
    explicit SceneEngine(QObject *parent = nullptr);
    SceneEngine(const SceneEngine&) = delete;
    SceneEngine& operator=(const SceneEngine&) = delete;

    void executeProcessingPipeline(const QString& input);
    void handleSceneMatchResult(const QJsonObject& result);
    void executeActionsWithSafety(const QList<QVariantMap>& actions);
};

#endif // SCENEENGINE_H
