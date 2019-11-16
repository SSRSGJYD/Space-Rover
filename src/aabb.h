#ifndef AABB_H
#define AABB_H

#include <QMatrix4x4>
#include <QOpenGLFunctions_3_3_Core>

class Intersection;

class AABB
{
public:
    AABB();
    AABB(QVector4D minPoint, QVector4D maxPoint);
    ~AABB();

    void transform(QMatrix4x4 matrix);
    bool pointInAABB(QVector3D p);
    bool intersect(Intersection& intersection);

public:
    float xmin, xmax;
    float ymin, ymax;
    float zmin, zmax;
};

#endif // AABB_H
