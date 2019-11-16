#include "geometry.h"


bool intersectPlane(Ray &ray, Point &p1, Point &p2, Point &p3, float &alpha, float &beta, float &gamma)
{
    Vector v0To1 = p2 - p1;
    Vector v0To2 = p3 - p1;
    Vector gnormal = cross(v0To1, v0To2);
    float det = -dot(ray.m_direction, gnormal);
    if (det == 0.0f)
        return false;

    Vector rOriginToV0 = p1 - ray.m_origin;
    Vector rayVertCross = cross(ray.m_direction, rOriginToV0);
    Vector rOriginToV1 = p2 - ray.m_origin;
    float invDet = 1.0f / det;

    // Calculate barycentric gamma coord
    gamma = -dot(rOriginToV1, rayVertCross) * invDet;

    Vector rOriginToV2 = p3 - ray.m_origin;

    // Calculate barycentric beta coord
    beta = dot(rOriginToV2, rayVertCross) * invDet;
    alpha = 1.0f - beta - gamma;
    return true;
}

bool intersectTriangle(Ray &ray, Point &p1, Point &p2, Point &p3)
{
    float alpha, beta, gamma;
    if(intersectPlane(ray, p1, p2, p3, alpha, beta, gamma)){
        if(alpha > 0 && beta > 0 && gamma > 0)
            return true;
    }
    return false;
}

bool intersectRectangle(Ray &ray, Point &p1, Point &p2, Point &p3, Point &p4)
{
    if(intersectTriangle(ray,p1,p2,p3) || intersectTriangle(ray, p1, p3, p4))
        return true;
    else
        return false;
}
