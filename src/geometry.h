#ifndef GEOMETRY_H
#define GEOMETRY_H

#include "raytracing/basic.h"

// Moller-Trumbore ray-triangle intersection test.  The point here is to
// find the barycentric coordinates of the triangle where the ray hits
// the plane the triangle lives in.  If the barycentric coordinates
// alpha, beta, gamma all add up to 1 (and each is in the 0.0 to 1.0
// range) then we have a valid intersection in the triangle.  Then, each
// of alpha, beta, gamma are the amounts of influence each vertex has
// on the values at the intersection.  So if we store things at the
// vertices (like normals, UVs, colors, etc) we can just weight them
// with the barycentric coordinates to get the interpolated result.
bool intersectPlane(Ray& ray,
                       Point& p1, Point& p2, Point& p3,
                       float& alpha, float& beta, float& gamma);
bool intersectTriangle(Ray& ray, Point& p1, Point& p2, Point& p3);
bool intersectRectangle(Ray& ray, Point& p1, Point& p2, Point& p3, Point& p4);

#endif // GEOMETRY_H
