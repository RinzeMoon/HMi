#ifndef SCENEDATATYPES_H
#define SCENEDATATYPES_H

#include <QString>
#include <QVariantMap>
#include <QList>

struct Action {
    QString actionId;
    QString actionType;
    QVariantMap parameters;
    int order;
    bool confirmRequired;
    int retryCount;
    int timeout;
    bool critical;
};

struct Scene {
    QString id;
    QString name;
    QStringList triggers;
    double confidenceThreshold;
    double autoConfirmThreshold;
    int priority;
    QList<Action> actions;
};

struct SceneMatchResult {
    QString primaryScene;
    QString sceneName;
    double confidence;
    QList<QVariantMap> alternativeScenes;
    QList<Action> actionList;
    QString explanation;
    bool isValid;
};

struct ConfirmationDecision {
    enum Type {
        AUTO_EXECUTE,
        REQUEST_CONFIRM,
        REQUEST_CLARIFICATION
    };
    
    Type type;
    QString message;
};

#endif // SCENEDATATYPES_H
