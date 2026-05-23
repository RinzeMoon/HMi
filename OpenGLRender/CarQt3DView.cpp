#include "CarQt3DView.h"
#include <QUrl>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>
#include <QFile>
#include <QFileInfo>
#include <QTemporaryFile>
#include <QTextStream>

CarQt3DEntity::CarQt3DEntity(Qt3DCore::QNode *parent)
    : Qt3DCore::QEntity(parent)
    , m_carEntity(nullptr)
    , m_carTransform(nullptr)
    , m_autoRotate(true)
    , m_rotationAngle(0.0f)
    , m_rotationTimer(nullptr)
{
    m_carEntity = new Qt3DCore::QEntity(this);

    m_carTransform = new Qt3DCore::QTransform();
    m_carTransform->setScale(5.0f);
    m_carTransform->setRotation(QQuaternion::fromAxisAndAngle(QVector3D(0, 1, 0), 0));
    m_carEntity->addComponent(m_carTransform);

    m_rotationTimer = new QTimer(this);
    connect(m_rotationTimer, &QTimer::timeout, this, [this]() {
        if (m_autoRotate) {
            m_rotationAngle += 0.5f;
            if (m_rotationAngle > 360.0f)
                m_rotationAngle -= 360.0f;
            m_carTransform->setRotation(QQuaternion::fromAxisAndAngle(QVector3D(0, 1, 0), m_rotationAngle));
        }
    });
    m_rotationTimer->start(16);
}

CarQt3DEntity::~CarQt3DEntity()
{
}

void CarQt3DEntity::setModelSource(const QString &source)
{
    if (!m_carEntity)
        return;

    foreach (Qt3DCore::QComponent *comp, m_carEntity->components()) {
        if (qobject_cast<Qt3DRender::QMesh*>(comp) || qobject_cast<Qt3DRender::QSceneLoader*>(comp) || 
            qobject_cast<Qt3DExtras::QPhongMaterial*>(comp) || qobject_cast<Qt3DExtras::QTextureMaterial*>(comp)) {
            m_carEntity->removeComponent(comp);
            comp->deleteLater();
        }
    }

    qDebug() << "CarQt3DEntity: loading mesh from:" << source;

    bool isGLTF = source.endsWith(".gltf") || source.endsWith(".glb");
    
    if (isGLTF) {
        QString filePath = source;
        if (source.startsWith("qrc:/")) {
            filePath = source.mid(4);
        } else if (source.startsWith("file:///")) {
            filePath = source.mid(8);
        }

        qDebug() << "CarQt3DEntity: GLTF/GLB file detected, path:" << filePath;
        qDebug() << "CarQt3DEntity: File exists:" << QFile::exists(filePath);

        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(filePath.toStdString(), 
            aiProcess_Triangulate | 
            aiProcess_GenNormals | 
            aiProcess_CalcTangentSpace |
            aiProcess_FlipUVs);

        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
            qDebug() << "CarQt3DEntity: Assimp import error:" << importer.GetErrorString();
            return;
        }

        qDebug() << "CarQt3DEntity: Assimp loaded successfully, meshes:" << scene->mNumMeshes;
        qDebug() << "CarQt3DEntity: materials:" << scene->mNumMaterials;

        if (scene->mNumMeshes > 0) {
            const aiMesh* aiMesh = scene->mMeshes[0];
            
            QVector<QVector3D> vertices;
            QVector<QVector3D> normals;
            QVector<QVector2D> texCoords;
            QVector<unsigned int> indices;

            for (unsigned int i = 0; i < aiMesh->mNumVertices; i++) {
                const aiVector3D& pos = aiMesh->mVertices[i];
                vertices.append(QVector3D(pos.x, pos.y, pos.z));

                if (aiMesh->mNormals) {
                    const aiVector3D& normal = aiMesh->mNormals[i];
                    normals.append(QVector3D(normal.x, normal.y, normal.z));
                }

                if (aiMesh->mTextureCoords[0]) {
                    const aiVector3D& tex = aiMesh->mTextureCoords[0][i];
                    texCoords.append(QVector2D(tex.x, tex.y));
                }
            }

            for (unsigned int i = 0; i < aiMesh->mNumFaces; i++) {
                const aiFace& face = aiMesh->mFaces[i];
                for (unsigned int j = 0; j < face.mNumIndices; j++) {
                    indices.append(face.mIndices[j]);
                }
            }

            qDebug() << "CarQt3DEntity: Loaded mesh - vertices:" << vertices.size() << "faces:" << indices.size();

            QString objPath = "D:/QtWorks/CarHMI/cmake-build-debug-visual-studio2/Debug/temp_model.obj";
            QFile objFile(objPath);
            if (objFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
                QTextStream out(&objFile);
                
                out << "# Converted from GLTF/GLB by Assimp\n";
                out << "o car_mesh\n";
                
                for (const QVector3D& v : vertices) {
                    out << "v " << v.x() << " " << v.y() << " " << v.z() << "\n";
                }
                
                for (const QVector3D& n : normals) {
                    out << "vn " << n.x() << " " << n.y() << " " << n.z() << "\n";
                }
                
                for (const QVector2D& t : texCoords) {
                    out << "vt " << t.x() << " " << t.y() << "\n";
                }
                
                out << "s off\n";
                
                for (int i = 0; i < indices.size(); i += 3) {
                    if (i + 2 < indices.size()) {
                        unsigned int i0 = indices[i] + 1;
                        unsigned int i1 = indices[i + 1] + 1;
                        unsigned int i2 = indices[i + 2] + 1;
                        
                        if (normals.size() > 0 && texCoords.size() > 0) {
                            out << "f " << i0 << "/" << i0 << "/" << i0 
                                << " " << i1 << "/" << i1 << "/" << i1 
                                << " " << i2 << "/" << i2 << "/" << i2 << "\n";
                        } else if (normals.size() > 0) {
                            out << "f " << i0 << "//" << i0 
                                << " " << i1 << "//" << i1 
                                << " " << i2 << "//" << i2 << "\n";
                        } else {
                            out << "f " << i0 << " " << i1 << " " << i2 << "\n";
                        }
                    }
                }
                
                objFile.close();
                
                qDebug() << "CarQt3DEntity: OBJ file created:" << objPath;
                qDebug() << "CarQt3DEntity: OBJ file size:" << QFileInfo(objPath).size() << "bytes";
                
                if (QFileInfo(objPath).size() == 0) {
                    qDebug() << "CarQt3DEntity: ERROR: OBJ file is empty!";
                    return;
                }
                
                Qt3DRender::QMesh *mesh = new Qt3DRender::QMesh();
                mesh->setSource(QUrl::fromLocalFile(objPath));
                mesh->setMeshName("car_mesh");
                m_carEntity->addComponent(mesh);

                Qt3DExtras::QPhongMaterial *material = new Qt3DExtras::QPhongMaterial();
                material->setDiffuse(QColor(100, 100, 100));
                material->setAmbient(QColor(50, 50, 50));
                material->setSpecular(QColor(200, 200, 200));
                material->setShininess(50.0f);
                m_carEntity->addComponent(material);

                qDebug() << "CarQt3DEntity: Assimp mesh added with default gray material";
                qDebug() << "CarQt3DEntity: Entity components:" << m_carEntity->components().size();
            } else {
                qDebug() << "CarQt3DEntity: ERROR: Failed to open OBJ file for writing:" << objPath;
            }
        }
    } else {
        Qt3DRender::QMesh *mesh = new Qt3DRender::QMesh();
        mesh->setSource(QUrl(source));
        mesh->setMeshName("car_mesh");
        m_carEntity->addComponent(mesh);

        connect(mesh, &Qt3DRender::QMesh::statusChanged, this, [mesh](Qt3DRender::QMesh::Status status) {
            qDebug() << "CarQt3DEntity: mesh status changed:" << status;
            if (status == Qt3DRender::QMesh::Error) {
                qDebug() << "CarQt3DEntity: Failed to load mesh from:" << mesh->source();
            }
        });

        Qt3DExtras::QPhongMaterial *material = new Qt3DExtras::QPhongMaterial();
        material->setDiffuse(QColor(100, 100, 100));
        material->setAmbient(QColor(50, 50, 50));
        material->setSpecular(QColor(200, 200, 200));
        material->setShininess(50.0f);
        m_carEntity->addComponent(material);

        qDebug() << "CarQt3DEntity: mesh added with default gray material";
        qDebug() << "CarQt3DEntity: entity has" << m_carEntity->components().size() << "components";

        QTimer::singleShot(1000, this, [mesh]() {
            qDebug() << "CarQt3DEntity: mesh status:" << mesh->status();
            qDebug() << "CarQt3DEntity: mesh source:" << mesh->source();
        });
    }
}

void CarQt3DEntity::setAutoRotate(bool autoRotate)
{
    m_autoRotate = autoRotate;
}

void CarQt3DEntity::setScale(float scale)
{
    if (m_carTransform) {
        m_carTransform->setScale(scale);
        qDebug() << "CarQt3DEntity: Scale set to:" << scale;
    }
}

CarQt3DView::CarQt3DView()
    : QQuickItem()
    , m_autoRotate(true)
    , m_modelPath("")
    , m_3dWindow(nullptr)
    , m_carEntity(nullptr)
    , m_3dInitialized(false)
{
    setFlag(QQuickItem::ItemHasContents, true);
    qDebug() << "CarQt3DView::CarQt3DView()";
    
    // 延迟初始化3D窗口
    QTimer::singleShot(500, this, [this]() {
        if (!m_3dInitialized) {
            qDebug() << "CarQt3DView: Forcing 3D initialization";
            init3D();
        }
    });
}

CarQt3DView::~CarQt3DView()
{
    if (m_3dWindow) {
        m_3dWindow->deleteLater();
    }
}

void CarQt3DView::setAutoRotate(bool enabled)
{
    if (m_autoRotate != enabled) {
        m_autoRotate = enabled;
        if (m_carEntity) {
            m_carEntity->setAutoRotate(enabled);
        }
        emit autoRotateChanged();
    }
}

void CarQt3DView::setModelPath(const QString &path)
{
    if (m_modelPath != path) {
        m_modelPath = path;
        if (m_carEntity) {
            m_carEntity->setModelSource(path);
        }
        emit modelPathChanged();
    }
}

void CarQt3DView::itemChange(ItemChange change, const ItemChangeData &value)
{
    QQuickItem::itemChange(change, value);

    if (change == ItemVisibleHasChanged) {
        qDebug() << "CarQt3DView::itemChange - visible:" << value.boolValue;
        if (value.boolValue && !m_3dInitialized) {
            QTimer::singleShot(100, this, &CarQt3DView::init3D);
        }
        if (m_3dWindow) {
            if (value.boolValue) {
                m_3dWindow->show();
                update3DWindowGeometry();
            } else {
                m_3dWindow->hide();
            }
        }
    }
    else if (change == ItemSceneChange) {
        if (value.window && !m_3dInitialized) {
            connect(value.window, &QQuickWindow::widthChanged, this, &CarQt3DView::update3DWindowGeometry);
            connect(value.window, &QQuickWindow::heightChanged, this, &CarQt3DView::update3DWindowGeometry);
        }
    }
}

void CarQt3DView::init3D()
{
    if (m_3dInitialized)
        return;

    qDebug() << "CarQt3DView::init3D() - size:" << width() << "x" << height();

    m_3dWindow = new Qt3DExtras::Qt3DWindow();
    m_3dWindow->setGeometry(0, 0, width(), height());

    Qt3DCore::QEntity *sceneRoot = new Qt3DCore::QEntity();

    m_carEntity = new CarQt3DEntity(sceneRoot);
        if (!m_modelPath.isEmpty()) {
            m_carEntity->setModelSource(m_modelPath);
        }
        m_carEntity->setAutoRotate(m_autoRotate);
        
        // 增加模型缩放
        m_carEntity->setScale(500.0f);

    Qt3DRender::QCamera *camera = m_3dWindow->camera();
    camera->setPosition(QVector3D(0, 2, 2));
    camera->setViewCenter(QVector3D(0, 0, 0));
    camera->setUpVector(QVector3D(0, 1, 0));
    qDebug() << "CarQt3DView: Camera position set to:" << camera->position();
    qDebug() << "CarQt3DView: Camera view center set to:" << camera->viewCenter();

    Qt3DExtras::QOrbitCameraController *camController = new Qt3DExtras::QOrbitCameraController(sceneRoot);
    camController->setCamera(camera);

    // 添加光源
    Qt3DRender::QDirectionalLight *light = new Qt3DRender::QDirectionalLight();
    light->setColor(QColor(255, 255, 255));
    light->setIntensity(2.0f);
    light->setWorldDirection(QVector3D(0, -1, -1));
    sceneRoot->addComponent(light);

    m_3dWindow->setRootEntity(sceneRoot);
    m_3dInitialized = true;

    qDebug() << "CarQt3DView: 3D window created, root entity set";
    qDebug() << "CarQt3DView: camera position:" << camera->position();
    qDebug() << "CarQt3DView: camera view center:" << camera->viewCenter();

    update3DWindowGeometry();
    m_3dWindow->show();
}

void CarQt3DView::update3DWindowGeometry()
{
    if (!m_3dWindow || !window())
        return;

    // 只有在可见时才更新几何和显示
    if (!isVisible()) {
        m_3dWindow->hide();
        return;
    }

    QRectF rect = mapRectToScene(QRectF(0, 0, width(), height()));
    QPoint screenPos = window()->mapToGlobal(rect.topLeft().toPoint());
    m_3dWindow->setGeometry(screenPos.x(), screenPos.y(), width(), height());
    m_3dWindow->show();
    qDebug() << "CarQt3DView: 3D window geometry updated:" << screenPos << "size:" << width() << "x" << height();
}