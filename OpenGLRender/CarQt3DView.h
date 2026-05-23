#ifndef CARQT3DVIEW_H
#define CARQT3DVIEW_H

#include <Qt3DCore/QEntity>
#include <Qt3DCore/QTransform>
#include <Qt3DRender/QMesh>
#include <Qt3DRender/QCamera>
#include <Qt3DRender/QTexture>
#include <Qt3DExtras/Qt3DWindow>
#include <Qt3DExtras/QOrbitCameraController>
#include <Qt3DExtras/QPhongMaterial>
#include <Qt3DExtras/QTextureMaterial>
#include <Qt3DRender/QDirectionalLight>
#include <Qt3DRender/QRenderStateSet>
#include <Qt3DRender/QPolygonOffset>
#include <Qt3DRender/QDepthTest>
#include <Qt3DRender/QSceneLoader>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>
#include <QQuickItem>
#include <QQuickWindow>
#include <QTimer>
#include <QDebug>

class CarQt3DEntity : public Qt3DCore::QEntity
{
    Q_OBJECT
public:
    CarQt3DEntity(Qt3DCore::QNode *parent = nullptr);
    ~CarQt3DEntity();

    void setModelSource(const QString &source);
    void setAutoRotate(bool autoRotate);
    void setScale(float scale);

private:
    Qt3DCore::QEntity *m_carEntity;
    Qt3DCore::QTransform *m_carTransform;
    bool m_autoRotate;
    float m_rotationAngle;
    QTimer *m_rotationTimer;
};

class CarQt3DView : public QQuickItem
{
    Q_OBJECT
    Q_PROPERTY(bool autoRotate READ autoRotate WRITE setAutoRotate NOTIFY autoRotateChanged)
    Q_PROPERTY(QString modelPath READ modelPath WRITE setModelPath NOTIFY modelPathChanged)

public:
    CarQt3DView();
    ~CarQt3DView();

    bool autoRotate() const { return m_autoRotate; }
    void setAutoRotate(bool enabled);

    QString modelPath() const { return m_modelPath; }
    void setModelPath(const QString &path);

signals:
    void autoRotateChanged();
    void modelPathChanged();

protected:
    void itemChange(ItemChange change, const ItemChangeData &value) override;

private:
    void init3D();
    void update3DWindowGeometry();

    bool m_autoRotate;
    QString m_modelPath;
    Qt3DExtras::Qt3DWindow *m_3dWindow;
    CarQt3DEntity *m_carEntity;
    bool m_3dInitialized;
};

#endif