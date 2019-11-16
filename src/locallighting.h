#ifndef LOCALLIGHTING_H
#define LOCALLIGHTING_H

#include <vector>
#include <QVector3D>
#include "raytracing/basic.h"

class Light;
class Texture;
class Scene;

bool inShadow(Point& intersect, Scene& scene, Light* light);
Color PhongLighting(Point& intersect,
                    Vector& normal,
                    const Vector& direction,
                    Scene& scene,
                    QVector3D ka, QVector3D ks, double n);

#endif // LOCALLIGHTING_H
