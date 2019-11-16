#ifndef LIGHT_H
#define LIGHT_H

#include "raytracing/basic.h"

class Light
{
public:
    enum LightType {LightBase, PointLight, ParallelLight};
    LightType m_type;
    Color m_color;

    Light(): m_type(LightBase){}
    Light(Color color): m_type(LightBase), m_color(color){}
    ~Light(){}

};

class PointLight: public Light
{
public:
    Point m_position;

    PointLight(): Light(){m_type=Light::PointLight;}
    PointLight(Color color, Point position): Light(color), m_position(position){m_type=Light::PointLight;}
    ~PointLight(){}
};

class ParallelLight: public Light
{
public:
    Vector m_direction;

    ParallelLight(Color color, Vector direction): Light(color), m_direction(direction.normalized()){m_type=Light::ParallelLight;}
    ~ParallelLight(){}
};

#endif // LIGHT_H
