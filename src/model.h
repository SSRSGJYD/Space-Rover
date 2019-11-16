#ifndef MODEL_H
#define MODEL_H

#include <vector>
#include <QOpenGLFunctions_3_3_Core>
#include <QOpenGLShaderProgram>

#include "aabb.h"

class Intersection;
class Color;

class Model
{
public:
    enum ModelType {MODEL, OBJMODEL};
    ModelType type;

    // phong parameters
    QVector3D ka;
    QVector3D ks;
    QVector3D kt;
    double n;

    // model matrix
    QMatrix4x4 modelMatrix;

    // AABB
    AABB aabb;

    Model(){
        type = MODEL;
    }
    ~Model(){}
    void Draw(QOpenGLShaderProgram& shader, QOpenGLFunctions_3_3_Core &core){}
    virtual bool intersect(Intersection& intersection) = 0;
};

#endif // MODEL_H
