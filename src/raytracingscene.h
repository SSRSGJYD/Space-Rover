#ifndef RAYTRACINGSCENE_H
#define RAYTRACINGSCENE_H

#include <QThread>
#include "raytracing/basic.h"

class RayTracingCamera
{
public:
    RayTracingCamera();
    virtual ~RayTracingCamera();
    virtual Ray makeRay(float xScreen, float yScreen) const = 0;
};


class PerspectiveCamera : public RayTracingCamera
{
public:
    PerspectiveCamera();
    PerspectiveCamera(float fov,
                      Point origin,
                      Vector front,
                      Vector targetUpDirection,
                      float nearDistance,
                      float focalDistance);

    ~PerspectiveCamera();
    void modifyPose(float fov, Point origin, Vector front, Vector targetUpDirection);
    Ray makeRay(float xScreen, float yScreen) const;

protected:
    Point m_origin;
    Vector m_forward;
    Vector m_right;
    Vector m_up;
    float m_tanFov;
    float m_nearDistance;
    float m_focalDistance;
};

class Image
{
public:
    Image(size_t width, size_t height)
        : m_width(width), m_height(height), m_pixels(new Color[width * height]) { }

    virtual ~Image() { delete[] m_pixels; }

    size_t width()  const { return m_width; }
    size_t height() const { return m_height; }

    Color& pixel(size_t x, size_t y)
    {
        return m_pixels[y * m_width + x];
    }

protected:
    size_t m_width, m_height;
    Color *m_pixels;
};

class Scene;
class Light;
class RayTracingCamera;

class RenderThread : public QThread
{
public:
    RenderThread(size_t xstart, size_t xend, size_t ystart, size_t yend,
                 Image *pImage,
                 Scene& masterSet,
                 const RayTracingCamera& cam,
                 unsigned int maxRayDepth);

protected:
    virtual void run();

    size_t m_xstart, m_xend, m_ystart, m_yend;
    Image *m_pImage;
    Scene& m_masterSet;
    const RayTracingCamera& m_camera;
    unsigned int m_maxRayDepth;
};

Color traceRay(const Ray& ray, Scene& scene, unsigned int maxRayDepth);

Image* renderRayTracing(Scene& scene,
                const RayTracingCamera& cam,
                size_t width,
                size_t height,
                unsigned int maxRayDepth);

#endif // RAYTRACINGSCENE_H
