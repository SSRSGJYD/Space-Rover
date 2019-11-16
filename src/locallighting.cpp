#include "locallighting.h"

#include <algorithm>
#include "raytracing/basic.h"
#include "light.h"
#include "mesh.h"
#include "scene.h"


Color PhongLighting(Point &intersect,
                    Vector& normal,
                    const Vector &direction,
                    Scene& scene,
                    QVector3D ka, QVector3D ks, double n)
{
    Color color;
    size_t light_num = scene.lights.size();
    for(size_t i=0; i<light_num; i++){
        if(!scene.lightFlag[i]) continue;
        if(inShadow(intersect, scene, scene.lights[i])) continue;
        // ambient
        Color ambient = ka * scene.lights[i]->m_color;
        // diffuse
        // assume that direction and normal is already normalized
        Vector light_direction;
        if(scene.lights[i]->m_type == Light::PointLight){
            PointLight* pointLight = (PointLight*)scene.lights[i];
            light_direction = pointLight->m_position - intersect;
            light_direction = light_direction.normalized();
        }
        else{ // parallel light
            ParallelLight* parallelLight = (ParallelLight*)scene.lights[i];
            light_direction = parallelLight->m_direction;
        }
        float diff = dot(normal, light_direction);
        diff = diff < 0.0f ? 0.0f : diff;
        Color diffuse = diff * scene.lights[i]->m_color;
        // specular
        Vector reflection = reflect(light_direction, normal);
        Vector view(-direction);
        float spec = dot(view, reflection);
        spec = spec < 0.0f ? 0.0f : spec;
        spec = float(pow(double(spec), n));
        Color specular = spec * ks * scene.lights[i]->m_color;
        color += ambient + diffuse + specular;
    }
    return color;
}

bool inShadow(Point &intersect, Scene &scene, Light *light)
{
    if(light->m_type == Light::PointLight){
        PointLight *pointLight = (PointLight*)light;
        Vector direction = intersect - pointLight->m_position;
        direction = direction.normalized();
        Ray lightRay(pointLight->m_position, direction);
        Intersection intersection(lightRay);
        scene.intersect(intersection);
        return !(intersection.m_intersect == intersect);
    }
    else{
        // parallel light
        ParallelLight *parallelLight = (ParallelLight*)light;
        Vector direction(-parallelLight->m_direction);
        Ray lightRay(intersect, direction);
        Intersection intersection(lightRay);
        return scene.intersect(intersection);
    }
}
