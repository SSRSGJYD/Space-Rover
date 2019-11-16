#include "aabb.h"
#include "geometry.h"

AABB::AABB()
{
    xmin = ymin = zmin = 1.0e10f;
    xmax = ymax = zmax = -1.0e10f;
}

AABB::AABB(QVector4D minPoint, QVector4D maxPoint)
{
    xmin = minPoint.x();
    ymin = minPoint.y();
    zmin = minPoint.z();
    xmax = maxPoint.x();
    ymax = maxPoint.y();
    zmax = maxPoint.z();
}

AABB::~AABB()
{

}

void AABB::transform(QMatrix4x4 matrix)
{
    QVector4D minPoint(xmin, ymin, zmin, 1.0f);
    QVector4D maxPoint(xmax, ymax, zmax, 1.0f);
    minPoint = matrix * minPoint;
    maxPoint = matrix * maxPoint;
    xmin = minPoint.x();
    ymin = minPoint.y();
    zmin = minPoint.z();
    xmax = maxPoint.x();
    ymax = maxPoint.y();
    zmax = maxPoint.z();
    if(xmin > xmax) {
        float tmp = xmin;
        xmin = xmax;
        xmax = tmp;
    }
    if(ymin > ymax) {
        float tmp = ymin;
        ymin = ymax;
        ymax = tmp;
    }
    if(zmin > zmax) {
        float tmp = zmin;
        zmin = zmax;
        zmax = tmp;
    }
}

bool AABB::pointInAABB(QVector3D p)
{
    if(xmin <= p.x() && xmax >= p.x()
            && ymin <= p.y() && ymax >= p.y()
            && zmin <= p.z() && zmax >= p.z())
        return true;
    else
        return false;
}

bool AABB::intersect(Intersection &intersection)
{
    Point p000(xmin, ymin, zmin);
    Point p001(xmin, ymin, zmax);
    Point p010(xmin, ymax, zmin);
    Point p011(xmin, ymax, zmax);
    Point p100(xmax, ymin, zmin);
    Point p101(xmax, ymin, zmax);
    Point p110(xmax, ymax, zmin);
    Point p111(xmax, ymax, zmax);
    return intersectRectangle(intersection.m_ray, p000, p001, p011, p010)
        || intersectRectangle(intersection.m_ray, p100, p101, p111, p110)
        || intersectRectangle(intersection.m_ray, p000, p010, p110, p100)
        || intersectRectangle(intersection.m_ray, p001, p011, p111, p101)
        || intersectRectangle(intersection.m_ray, p000, p001, p101, p100)
        || intersectRectangle(intersection.m_ray, p010, p011, p111, p110);
}
