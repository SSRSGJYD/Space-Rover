#include "scene.h"
#include <QJsonDocument>
#include <QJsonParseError>
#include <QFile>
#include <QJsonObject>
#include <QDebug>
#include <QJsonArray>
#include <QDir>
#include "objmodel.h"
#include "light.h"

Scene::Scene()
{

}

Scene::Scene(const char* sceneFile, QOpenGLFunctions_3_3_Core *pCore){
    this->sceneFile = QString(sceneFile);
    this->pCore = pCore;
}

void Scene::loadScene()
{
    QFile file(sceneFile);
    if(!file.open(QIODevice::ReadOnly))
    {
        qDebug() << "could't open projects json";
    }
    QByteArray allData = file.readAll();
    file.close();
    QJsonParseError json_error;
    QJsonDocument jsonDoc(QJsonDocument::fromJson(allData, &json_error));
    if(json_error.error != QJsonParseError::NoError)
    {
        qDebug() << "json error!";
    }
    QJsonObject rootObj = jsonDoc.object();
    // camera
    camera_yaw = float(rootObj["camera_yaw"].toDouble());
    camera_pitch = float(rootObj["camera_pitch"].toDouble());
    camera_nearplane = float(rootObj["camera_nearplane"].toDouble());
    camera_farplane = float(rootObj["camera_farplane"].toDouble());
    camera_position = loadQVector3D(rootObj["camera_position"].toObject());
    camera_up = loadQVector3D(rootObj["camera_up"].toObject());
    camera_zoom = float(rootObj["camera_zoom"].toDouble());
    camera_nearDistance = float(rootObj["camera_nearDistance"].toDouble());
    camera_focalDistance = float(rootObj["camera_focalDistance"].toDouble());
    // lights
    QJsonArray lightArray = rootObj["lights"].toArray();
    foreach(QJsonValue value, lightArray){
        QJsonObject lightObj = value.toObject();
        Light* light = loadLight(lightObj);
        lights.push_back(light);
        lightFlag.push_back(true);
    }
    // models
    QJsonArray modelArray = rootObj["models"].toArray();
    foreach(QJsonValue value, modelArray){
        QJsonObject modelObj = value.toObject();
        Model* model = loadModel(modelObj);
        models.push_back(model);
    }
}

void Scene::loadModels()
{
    QFile file(sceneFile);
    if(!file.open(QIODevice::ReadOnly))
    {
        qDebug() << "could't open projects json";
    }
    QByteArray allData = file.readAll();
    file.close();
    QJsonParseError json_error;
    QJsonDocument jsonDoc(QJsonDocument::fromJson(allData, &json_error));
    if(json_error.error != QJsonParseError::NoError)
    {
        qDebug() << "json error!";
    }
    QJsonObject rootObj = jsonDoc.object();
    // models
    QJsonArray modelArray = rootObj["models"].toArray();
    foreach(QJsonValue value, modelArray){
        QJsonObject modelObj = value.toObject();
        Model* model = loadModel(modelObj);
        models.push_back(model);
    }
}

void Scene::loadSceneExceptModels()
{
    QFile file(sceneFile);
    if(!file.open(QIODevice::ReadOnly))
    {
        qDebug() << "could't open projects json";
    }
    QByteArray allData = file.readAll();
    file.close();
    QJsonParseError json_error;
    QJsonDocument jsonDoc(QJsonDocument::fromJson(allData, &json_error));
    if(json_error.error != QJsonParseError::NoError)
    {
        qDebug() << "json error!";
    }
    QJsonObject rootObj = jsonDoc.object();
    // camera
    camera_yaw = float(rootObj["camera_yaw"].toDouble());
    camera_pitch = float(rootObj["camera_pitch"].toDouble());
    camera_nearplane = float(rootObj["camera_nearplane"].toDouble());
    camera_farplane = float(rootObj["camera_farplane"].toDouble());
    camera_position = loadQVector3D(rootObj["camera_position"].toObject());
    camera_up = loadQVector3D(rootObj["camera_up"].toObject());
    camera_zoom = float(rootObj["camera_zoom"].toDouble());
    camera_nearDistance = float(rootObj["camera_nearDistance"].toDouble());
    camera_focalDistance = float(rootObj["camera_focalDistance"].toDouble());
    // lights
    QJsonArray lightArray = rootObj["lights"].toArray();
    foreach(QJsonValue value, lightArray){
        QJsonObject lightObj = value.toObject();
        Light* light = loadLight(lightObj);
        lights.push_back(light);
        lightFlag.push_back(true);
    }
}

QVector3D Scene::loadQVector3D(QJsonObject obj) {
    float x = float(obj["x"].toDouble());
    float y = float(obj["y"].toDouble());
    float z = float(obj["z"].toDouble());
    return QVector3D(x, y, z);
}

QJsonObject Scene::saveQVector3D(QVector3D v)
{
    QJsonObject obj;
    obj.insert("x", double(v.x()));
    obj.insert("y", double(v.y()));
    obj.insert("z", double(v.z()));
    return obj;
}

QMatrix4x4 Scene::loadQMatrix4x4(QJsonArray arrObj)
{
    float arr[16];
    for(int c=0; c<4; c++){
        for(int r=0; r<4; r++){
            arr[r*4+c] = float(arrObj.at(c*4+r).toDouble());
        }
    }
    QMatrix4x4 matrix(arr);
    return matrix;
}

QJsonArray Scene::saveQMatrix4x4(QMatrix4x4 v)
{
    float* arr = v.data();
    QJsonArray arrObj;
    for(int i=0; i<16; i++){
        arrObj.append(double(arr[i]));
    }
    return arrObj;
}

Light *Scene::loadLight(QJsonObject obj)
{
    int type = obj["type"].toInt();
    if(type == Light::PointLight){
        Color color(loadQVector3D(obj["light_color"].toObject()));
        Vector position(loadQVector3D(obj["light_position"].toObject()));
        PointLight* pointLight = new PointLight(color, position);
        return pointLight;
    }
    else if(type == Light::ParallelLight){
        Color color(loadQVector3D(obj["light_color"].toObject()));
        Vector direction(loadQVector3D(obj["light_direction"].toObject()));
        ParallelLight* parallelLight = new ParallelLight(color, direction);
        return parallelLight;
    }
}

QJsonObject Scene::saveLight(Light *light)
{
    QJsonObject obj;
    obj.insert("type", light->m_type);
    if(light->m_type == Light::PointLight){
        PointLight *pointLight = (PointLight*)light;
        obj.insert("light_color", saveQVector3D(pointLight->m_color.toQVector3D()));
        obj.insert("light_position", saveQVector3D(pointLight->m_position.toQVector3D()));
    }
    else if(light->m_type == Light::ParallelLight){
        ParallelLight *parallelLight = (ParallelLight*)light;
        obj.insert("light_color", saveQVector3D(parallelLight->m_color.toQVector3D()));
        obj.insert("light_direction", saveQVector3D(parallelLight->m_direction.toQVector3D()));
    }
    return obj;
}

Model* Scene::loadModel(QJsonObject obj){
    int type = obj["type"].toInt();
    if(type == Model::OBJMODEL){
        string objPath = obj["path"].toString().toStdString();
        ObjModel* model = new ObjModel(objPath, false, *pCore);
        model->modelMatrix = loadQMatrix4x4(obj["modelMatrix"].toArray());
        model->ka = loadQVector3D(obj["ka"].toObject());
        model->ks = loadQVector3D(obj["ks"].toObject());
        model->kt = loadQVector3D(obj["kt"].toObject());
        model->n = obj["n"].toDouble();
        return model;
    }
}

QJsonObject Scene::saveModel(Model *model)
{
    QJsonObject obj;
    obj.insert("type", model->type);
    if(model->type == Model::OBJMODEL){
        ObjModel *p = (ObjModel*)model;
        obj.insert("path", QString::fromStdString(p->path));
    }
    obj.insert("modelMatrix", saveQMatrix4x4(model->modelMatrix));
    obj.insert("ka", saveQVector3D(model->ka));
    obj.insert("ks", saveQVector3D(model->ks));
    obj.insert("kt", saveQVector3D(model->kt));
    obj.insert("n", double(model->n));
    return obj;
}

QJsonObject Scene::saveObjModel(std::string path, QMatrix4x4 modelMatrix, QVector3D ka, QVector3D ks, QVector3D kt, double n)
{
    QJsonObject obj;
    obj.insert("type", 1);
    obj.insert("path", QString::fromStdString(path));
    obj.insert("modelMatrix", saveQMatrix4x4(modelMatrix));
    obj.insert("ka", saveQVector3D(ka));
    obj.insert("ks", saveQVector3D(ks));
    obj.insert("kt", saveQVector3D(kt));
    obj.insert("n", n);
    return obj;
}

void Scene::saveScene(QString sceneFile)
{
    QFile file(QDir::homePath()+sceneFile);
    if(!file.open(QIODevice::ReadWrite)) {
        qDebug() << "File open error";
    }
    else {
        qDebug() <<"File open!";
    }
    // 清空文件中的原有内容
    file.resize(0);
    QJsonDocument jsonDoc;
    QJsonObject rootObj;
    // camera
    rootObj.insert("camera_yaw", double(camera_yaw));
    rootObj.insert("camera_pitch", double(camera_pitch));
    rootObj.insert("camera_nearplane", double(camera_nearplane));
    rootObj.insert("camera_farplane", double(camera_farplane));
    rootObj.insert("camera_position", saveQVector3D(camera_position));
    rootObj.insert("camera_up", saveQVector3D(camera_up));
    rootObj.insert("camera_zoom", double(camera_zoom));
    rootObj.insert("camera_nearDistance", double(camera_nearDistance));
    rootObj.insert("camera_focalDistance", double(camera_focalDistance));
    // lights
    QJsonArray lightArray;
    foreach(Light* light, lights){
        QJsonObject obj = saveLight(light);
        lightArray.append(obj);
    }
    rootObj.insert("lights", lightArray);
    // models
    QJsonArray modelArray;
    foreach(Model* model, models){
        QJsonObject obj = saveModel(model);
        modelArray.append(obj);
    }
    rootObj.insert("models", modelArray);

    jsonDoc.setObject(rootObj);
    file.write(jsonDoc.toJson());
    file.close();
}

void Scene::saveObjScene(QString sceneFile, std::string path, QMatrix4x4 modelMatrix, QVector3D ka, QVector3D ks, QVector3D kt, double n)
{
    QFile file(QDir::homePath()+sceneFile);
    if(!file.open(QIODevice::ReadWrite)) {
        qDebug() << "File open error";
    }
    else {
        qDebug() <<"File open!";
    }
    // 清空文件中的原有内容
    file.resize(0);
    QJsonDocument jsonDoc;
    QJsonObject rootObj;
    // camera
    rootObj.insert("camera_yaw", double(camera_yaw));
    rootObj.insert("camera_pitch", double(camera_pitch));
    rootObj.insert("camera_nearplane", double(camera_nearplane));
    rootObj.insert("camera_farplane", double(camera_farplane));
    rootObj.insert("camera_position", saveQVector3D(camera_position));
    rootObj.insert("camera_up", saveQVector3D(camera_up));
    rootObj.insert("camera_zoom", double(camera_zoom));
    rootObj.insert("camera_nearDistance", double(camera_nearDistance));
    rootObj.insert("camera_focalDistance", double(camera_focalDistance));
    // lights
    QJsonArray lightArray;
    foreach(Light* light, lights){
        QJsonObject obj = saveLight(light);
        lightArray.append(obj);
    }
    rootObj.insert("lights", lightArray);

    // models
    QJsonArray modelArray;
    QJsonObject obj = saveObjModel(path, modelMatrix, ka, ks, kt, n);
    modelArray.append(obj);
    rootObj.insert("models", modelArray);

    jsonDoc.setObject(rootObj);
    file.write(jsonDoc.toJson());
    file.close();
}

bool Scene::intersect(Intersection &intersection)
{
    size_t size = models.size();
    for(size_t i = 0; i < size; i++){
        if(models[i]->intersect(intersection)){
            intersection.intersected = true;
        }
    }
    return intersection.intersected;
}
