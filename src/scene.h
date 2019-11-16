#ifndef SCENE_H
#define SCENE_H

#include <vector>
#include <QString>
#include <QVector3D>
#include <QMatrix4x4>
#include <QOpenGLFunctions_3_3_Core>

class Model;
class Light;
class Intersection;

class Scene
{
public:
    QString sceneFile;
    QOpenGLFunctions_3_3_Core *pCore;

    // Default camera parameters
    float camera_yaw;
    float camera_pitch;
    float camera_nearplane;
    float camera_farplane;
    // for raytracing perspective camera
    float camera_nearDistance;
    float camera_focalDistance;

    // Default camera settings
    QVector3D camera_position;
    QVector3D camera_up;
    float camera_zoom;

    // lights
    std::vector<Light*> lights;
    std::vector<bool> lightFlag;

    // models
    std::vector<Model*> models;

public:
    Scene();
    Scene(const char* sceneFile, QOpenGLFunctions_3_3_Core *pCore);

    QVector3D loadQVector3D(QJsonObject obj);
    QJsonObject saveQVector3D(QVector3D v);
    QMatrix4x4 loadQMatrix4x4(QJsonArray arrObj);
    QJsonArray saveQMatrix4x4(QMatrix4x4 v);
    Light* loadLight(QJsonObject obj);
    QJsonObject saveLight(Light* light);
    Model* loadModel(QJsonObject obj);
    QJsonObject saveModel(Model* model);
    QJsonObject saveObjModel(std::string, QMatrix4x4, QVector3D, QVector3D, QVector3D, double);
    void loadScene();
    void loadSceneExceptModels();
    void loadModels();
    void saveScene(QString sceneFile);
    void saveObjScene(QString sceneFile, std::string, QMatrix4x4, QVector3D, QVector3D, QVector3D, double);

    bool intersect(Intersection& intersection);
};

#endif // SCENE_H
