#include "raytracingscene.h"

#include "raytracing/basic.h"
#include "model.h"
#include "scene.h"
#include "locallighting.h"


RayTracingCamera::RayTracingCamera() { }

RayTracingCamera::~RayTracingCamera()
{

}

PerspectiveCamera::PerspectiveCamera(){}

PerspectiveCamera::PerspectiveCamera(float fov,
                                     Point origin,
                                     Vector front,
                                     Vector targetUpDirection,
                                     float nearDistance,
                                     float focalDistance)
    : m_origin(origin),
      m_forward(front.normalized()),
      m_tanFov(std::tan(fov * M_PI / 180.0f)),
      m_nearDistance(nearDistance),
      m_focalDistance(focalDistance)
{
    m_right = cross(m_forward, targetUpDirection).normalized();
    m_up = cross(m_right, m_forward);
}

PerspectiveCamera::~PerspectiveCamera()
{

}

void PerspectiveCamera::modifyPose(float fov, Point origin, Vector front, Vector targetUpDirection)
{
    m_tanFov = std::tan(fov * M_PI / 180.0f);
    m_origin = origin;
    m_forward = front.normalized();
    m_right = cross(m_forward, targetUpDirection).normalized();
    m_up = cross(m_right, m_forward);
}

Ray PerspectiveCamera::makeRay(float xScreen, float yScreen) const
{
    Ray ray;
    ray.m_origin = m_origin;
    ray.m_direction = m_forward +
                      m_right * ((xScreen - 0.5f) * m_tanFov) +
                      m_up * ((yScreen - 0.5f) * m_tanFov);
    ray.m_direction.normalize();
    return ray;
}

RenderThread::RenderThread(size_t xstart, size_t xend, size_t ystart, size_t yend,
                 Image *pImage,
                 Scene& masterSet,
                 const RayTracingCamera& cam,
                 unsigned int maxRayDepth)
        : m_xstart(xstart), m_xend(xend), m_ystart(ystart), m_yend(yend),
          m_pImage(pImage), m_masterSet(masterSet), m_camera(cam),
          m_maxRayDepth(maxRayDepth) { }


void RenderThread::run()
{
    // The aspect ratio is used to make the image only get more zoomed in when
    // the height changes (and not the width)
    float aspectRatioXToY = float(m_pImage->width()) / float(m_pImage->height());

    // For each pixel row...
    for (size_t y = m_ystart; y < m_yend; ++y)
    {
        // For each pixel across the row...
        for (size_t x = m_xstart; x < m_xend; ++x)
        {
            // Accumulate pixel color
            Color pixelColor(0.0f, 0.0f, 0.0f);
            size_t totalPixelSamples = 1;
            // For each sample in the pixel...
            for (size_t psi = 0; psi < totalPixelSamples; ++psi)
            {
                float xu = x / float(m_pImage->width());
                // Flip pixel row to be in screen space (images are top-down)
                float yu = 1.0f - y / float(m_pImage->height());
                // Find where this pixel sample hits in the scene
                Ray ray = m_camera.makeRay((xu - 0.5f) * aspectRatioXToY + 0.5f, yu);
                // Trace a path out, gathering estimated radiance along the path
                pixelColor += traceRay(ray, m_masterSet, m_maxRayDepth);
            }
            // Divide by the number of pixel samples (a box pixel filter, essentially)
            pixelColor /= totalPixelSamples;
            // Store off the computed pixel in a big buffer
            m_pImage->pixel(x, y) = pixelColor;
        }
    }
}

Color traceRay(const Ray& ray, Scene& scene, unsigned int maxRayDepth)
{
    if(maxRayDepth == 0)
        return Color(0.0f, 0.0f, 0.0f);
    Intersection intersection(ray);
    if (!scene.intersect(intersection))
        return Color(0.0f, 0.0f, 0.0f);
    Color localLightingColor = PhongLighting(intersection.m_intersect,
                                             intersection.m_normal,
                                             ray.m_direction,
                                             scene,
                                             intersection.m_pModel->ka,
                                             intersection.m_pModel->ks,
                                             intersection.m_pModel->n);
    Vector reflection = reflect(ray.m_direction, intersection.m_normal);
    Ray reflectionRay(intersection.m_intersect, reflection);
    Color reflectionColor = traceRay(reflectionRay, scene, maxRayDepth-1);
    Ray transmissionRay(intersection.m_intersect, ray.m_direction);
    Color transmissionColor = traceRay(transmissionRay, scene, maxRayDepth-1);
    Color ks(intersection.m_pModel->ks);
    Color kt(intersection.m_pModel->kt);
    Color resultColor = localLightingColor + ks * reflectionColor + kt * transmissionColor;
    resultColor.clamp();
    return resultColor;
}

//Image* renderRayTracing(Scene& scene,
//                        const RayTracingCamera& cam,
//                        size_t width,
//                        size_t height,
//                        unsigned int maxRayDepth)
//{
//    // Set up the output image
//    Image *m_pImage = new Image(width, height);

//    // The aspect ratio is used to make the image only get more zoomed in when
//    // the height changes (and not the width)
//    float aspectRatioXToY = float(m_pImage->width()) / float(m_pImage->height());

//    // For each pixel row...
//    for (size_t y = 0; y < height; ++y)
//    {
//        // For each pixel across the row...
//        for (size_t x = 0; x < width; ++x)
//        {
//            // Accumulate pixel color
//            Color pixelColor(0.0f, 0.0f, 0.0f);
//            size_t totalPixelSamples = 1;
//            // For each sample in the pixel...
//            for (size_t psi = 0; psi < totalPixelSamples; ++psi)
//            {
//                float xu = x / float(m_pImage->width());
//                // Flip pixel row to be in screen space (images are top-down)
//                float yu = 1.0f - y / float(m_pImage->height());
//                // Find where this pixel sample hits in the scene
//                Ray ray = cam.makeRay((xu - 0.5f) * aspectRatioXToY + 0.5f, yu);
//                // Trace a path out, gathering estimated radiance along the path
//                pixelColor += traceRay(ray, scene, maxRayDepth);
//            }
//            // Divide by the number of pixel samples (a box pixel filter, essentially)
//            pixelColor /= totalPixelSamples;
//            // Store off the computed pixel in a big buffer
//            m_pImage->pixel(x, y) = pixelColor;
//        }
//    }

//    // We made a picture!
//    return m_pImage;
//}

Image* renderRayTracing(Scene& scene,
                        const RayTracingCamera& cam,
                        size_t width,
                        size_t height,
                        unsigned int maxRayDepth)
{
    // Set up the output image
    Image *pImage = new Image(width, height);

    // Set up render threads; we make as much as 16 chunks of the image that
    // can render in parallel.
    const size_t kChunkDim = 4;

    // Chunk size is the number of pixels per image chunk (we have to take care
    // to deal with tiny images)
    size_t xChunkSize = width >= kChunkDim ? width / kChunkDim : 1;
    size_t yChunkSize = height >= kChunkDim ? height / kChunkDim : 1;
    // Chunks are the number of chunks in each dimension we can chop the image
    // into (again, taking care to deal with tiny images, and also images that
    // don't divide clealy into 4 chunks)
    size_t xChunks = width > kChunkDim ? width / xChunkSize : 1;
    size_t yChunks = height > kChunkDim ? height / yChunkSize : 1;
    if (xChunks * xChunkSize < width) xChunks++;
    if (yChunks * yChunkSize < height) yChunks++;

    // Set up render threads
    size_t numRenderThreads = xChunks * yChunks;
    RenderThread **renderThreads = new RenderThread*[numRenderThreads];

    // Launch render threads
    for (size_t yc = 0; yc < yChunks; ++yc)
    {
        size_t yStart = yc * yChunkSize;
        size_t yEnd = std::min((yc + 1) * yChunkSize, height);
        for (size_t xc = 0; xc < xChunks; ++xc)
        {
            size_t xStart = xc * xChunkSize;
            size_t xEnd = std::min((xc + 1) * xChunkSize, width);
            renderThreads[yc * xChunks + xc] = new RenderThread(xStart,
                                                                xEnd,
                                                                yStart,
                                                                yEnd,
                                                                pImage,
                                                                scene,
                                                                cam,
                                                                maxRayDepth);
            renderThreads[yc * xChunks + xc]->start();
        }
    }

    // Wait until the render finishes
    bool stillRunning;
    do
    {
        // See if any render thread is still going
        stillRunning = false;
        for (size_t i = 0; i < numRenderThreads; ++i)
        {
            if (renderThreads[i]->isRunning())
            {
                stillRunning = true;
                break;
            }
        }
        if (stillRunning)
        {
            // Give up the CPU so the render threads can do their thing
            QThread::yieldCurrentThread();
        }
    } while (stillRunning);

    // Clean up render thread objects
    for (size_t i = 0; i < numRenderThreads; ++i)
    {
        delete renderThreads[i];
    }
    delete[] renderThreads;

    return pImage;
}

