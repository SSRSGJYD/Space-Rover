#ifndef RAYTRACING_BASIC_H
#define RAYTRACING_BASIC_H

#include <cmath>
#include <algorithm>
#include <vector>
#include <QVector3D>
#include <QMatrix4x4>
#include <glm/glm.hpp>

#ifndef M_PI
    #define M_PI 3.1415926
#endif

struct Color
{
    float m_r, m_g, m_b;

    Color()                          : m_r(0.0f), m_g(0.0f), m_b(0.0f)    { }
    Color(const Color& c)            : m_r(c.m_r), m_g(c.m_g), m_b(c.m_b) { }
    Color(float r, float g, float b) : m_r(r), m_g(g), m_b(b)             { }
    explicit Color(float f)          : m_r(f), m_g(f), m_b(f)             { }
    Color(QVector3D c)               : m_r(c.x()), m_g(c.y()), m_b(c.z()) {}

    QVector3D toQVector3D(){
        return QVector3D(m_r, m_g, m_b);
    }

    void clamp(float min = 0.0f, float max = 1.0f)
    {
        m_r = std::max(min, std::min(max, m_r));
        m_g = std::max(min, std::min(max, m_g));
        m_b = std::max(min, std::min(max, m_b));
    }


    Color& operator =(const Color& c)
    {
        m_r = c.m_r;
        m_g = c.m_g;
        m_b = c.m_b;
        return *this;
    }

    Color& operator +=(const Color& c)
    {
        m_r += c.m_r;
        m_g += c.m_g;
        m_b += c.m_b;
        return *this;
    }

    Color& operator -=(const Color& c)
    {
        m_r -= c.m_r;
        m_g -= c.m_g;
        m_b -= c.m_b;
        return *this;
    }

    Color& operator *=(const Color& c)
    {
        m_r *= c.m_r;
        m_g *= c.m_g;
        m_b *= c.m_b;
        return *this;
    }

    Color& operator /=(const Color& c)
    {
        m_r /= c.m_r;
        m_g /= c.m_g;
        m_b /= c.m_b;
        return *this;
    }

    Color& operator *=(float f)
    {
        m_r *= f;
        m_g *= f;
        m_b *= f;
        return *this;
    }

    Color& operator /=(float f)
    {
        m_r /= f;
        m_g /= f;
        m_b /= f;
        return *this;
    }
};


inline Color operator +(const Color& c1, const Color& c2)
{
    return Color(c1.m_r + c2.m_r,
                 c1.m_g + c2.m_g,
                 c1.m_b + c2.m_b);
}


inline Color operator -(const Color& c1, const Color& c2)
{
    return Color(c1.m_r - c2.m_r,
                 c1.m_g - c2.m_g,
                 c1.m_b - c2.m_b);
}


inline Color operator *(const Color& c1, const Color& c2)
{
    return Color(c1.m_r * c2.m_r,
                 c1.m_g * c2.m_g,
                 c1.m_b * c2.m_b);
}


inline Color operator /(const Color& c1, const Color& c2)
{
    return Color(c1.m_r / c2.m_r,
                 c1.m_g / c2.m_g,
                 c1.m_b / c2.m_b);
}


inline Color operator *(const Color& c, float f)
{
    return Color(f * c.m_r,
                 f * c.m_g,
                 f * c.m_b);
}


inline Color operator *(float f, const Color& c)
{
    return Color(f * c.m_r,
                 f * c.m_g,
                 f * c.m_b);
}


inline Color operator /(const Color& c, float f)
{
    return Color(c.m_r / f,
                 c.m_g / f,
                 c.m_b / f);
}


//
// 3D vector class (and associated operations)
//
// Operators supported:
//     vector = vector
//     -vector
//     vector + vector, vector += vector
//     vector - vector, vector -= vector
//     vector * float, vector *= float, float * vector
//     vector / float, vector /= float, float / vector
//

struct Vector
{
    float m_x, m_y, m_z;

    Vector()                          : m_x(0.0f), m_y(0.0f), m_z(0.0f)    { }
    Vector(const Vector& v)           : m_x(v.m_x), m_y(v.m_y), m_z(v.m_z) { }
    Vector(float x, float y, float z) : m_x(x), m_y(y), m_z(z)             { }
    explicit Vector(float f)          : m_x(f), m_y(f), m_z(f)             { }
    Vector(QVector3D v)               : m_x(v.x()), m_y(v.y()), m_z(v.z()) { }
    Vector(glm::vec3 v)               : m_x(v.x), m_y(v.y), m_z(v.z)       { }

    QVector3D toQVector3D(){
        return QVector3D(m_x, m_y, m_z);
    }

    float length2() const { return m_x * m_x + m_y * m_y + m_z * m_z; }
    float length()  const { return std::sqrt(length2()); }

    // Returns old length from before normalization (ignore the return value if you don't need it)
    float  normalize()        { float len = length(); if (len > 0) *this /= len; return len; }
    // Return a vector in this same direction, but normalized
    Vector normalized() const { Vector r(*this); r.normalize(); return r; }

    float maxComponent() const { return std::max(std::max(m_x, m_y), m_z); }
    float minComponent() const { return std::min(std::min(m_x, m_y), m_z); }


    Vector& operator =(const Vector& v)
    {
        m_x = v.m_x;
        m_y = v.m_y;
        m_z = v.m_z;
        return *this;
    }

    Vector& operator +=(const Vector& v)
    {
        m_x += v.m_x;
        m_y += v.m_y;
        m_z += v.m_z;
        return *this;
    }

    Vector& operator -=(const Vector& v)
    {
        m_x -= v.m_x;
        m_y -= v.m_y;
        m_z -= v.m_z;
        return *this;
    }

    Vector& operator *=(const Vector& v)
    {
        m_x *= v.m_x;
        m_y *= v.m_y;
        m_z *= v.m_z;
        return *this;
    }

    Vector& operator *=(float f)
    {
        m_x *= f;
        m_y *= f;
        m_z *= f;
        return *this;
    }

    Vector& operator /=(const Vector& v)
    {
        m_x /= v.m_x;
        m_y /= v.m_y;
        m_z /= v.m_z;
        return *this;
    }

    Vector& operator /=(float f)
    {
        m_x /= f;
        m_y /= f;
        m_z /= f;
        return *this;
    }

    Vector operator -() const
    {
        return Vector(-m_x, -m_y, -m_z);
    }

    bool operator == (const Vector& v)
    {
        return fabs(m_x-v.m_x) < 1e-6
                && fabs(m_y-v.m_y) < 1e-6
                && fabs(m_z-v.m_z) < 1e-6;
    }
};


inline Vector operator +(const Vector& v1, const Vector& v2)
{
    return Vector(v1.m_x + v2.m_x,
                  v1.m_y + v2.m_y,
                  v1.m_z + v2.m_z);
}


inline Vector operator -(const Vector& v1, const Vector& v2)
{
    return Vector(v1.m_x - v2.m_x,
                  v1.m_y - v2.m_y,
                  v1.m_z - v2.m_z);
}


inline Vector operator *(const Vector& v1, const Vector& v2)
{
    return Vector(v1.m_x * v2.m_x,
                  v1.m_y * v2.m_y,
                  v1.m_z * v2.m_z);
}


inline Vector operator *(const Vector& v, float f)
{
    return Vector(f * v.m_x,
                  f * v.m_y,
                  f * v.m_z);
}


inline Vector operator *(float f, const Vector& v)
{
    return Vector(f * v.m_x,
                  f * v.m_y,
                  f * v.m_z);
}


inline Vector operator /(const Vector& v1, const Vector& v2)
{
    return Vector(v1.m_x / v2.m_x,
                  v1.m_y / v2.m_y,
                  v1.m_z / v2.m_z);
}


inline Vector operator /(float f, const Vector& v)
{
    return Vector(f / v.m_x,
                  f / v.m_y,
                  f / v.m_z);
}


inline Vector operator /(const Vector& v, float f)
{
    return Vector(v.m_x / f,
                  v.m_y / f,
                  v.m_z / f);
}


// dot(v1, v2) = length(v1) * length(v2) * cos(angle between v1, v2)
inline float dot(const Vector& v1, const Vector& v2)
{
    // In cartesian coordinates, it simplifies to this simple calculation:
    return v1.m_x * v2.m_x + v1.m_y * v2.m_y + v1.m_z * v2.m_z;
}


// cross(v1, v2) = length(v1) * length(v2) * sin(angle between v1, v2);
// result is perpendicular to both v1, v2.
inline Vector cross(const Vector& v1, const Vector& v2)
{
    // In cartesian coordinates, it simplifies down to this calculation:
    return Vector(v1.m_y * v2.m_z - v1.m_z * v2.m_y,
                  v1.m_z * v2.m_x - v1.m_x * v2.m_z,
                  v1.m_x * v2.m_y - v1.m_y * v2.m_x);
}


inline Vector max(const Vector& v1, const Vector& v2)
{
    return Vector(std::max(v1.m_x, v2.m_x),
                  std::max(v1.m_y, v2.m_y),
                  std::max(v1.m_z, v2.m_z));
}

inline Vector min(const Vector& v1, const Vector& v2)
{
    return Vector(std::min(v1.m_x, v2.m_x),
                  std::min(v1.m_y, v2.m_y),
                  std::min(v1.m_z, v2.m_z));
}

inline Vector reflect(const Vector& direction, const Vector& normal)
{
    Vector view = -direction;
    Vector reflection = 2.0*normal*dot(normal, view)-view;
    return reflection.normalized();
}

typedef Vector Point;


const float kRayTMin = 0.0001f;
const float kRayTMax = 1.0e30f;

struct Ray
{
    Point m_origin;
    Vector m_direction;
    float m_tMax;
    float m_time;

    // Some sane defaults
    Ray()
        : m_origin(),
          m_direction(0.0f, 0.0f, 1.0f),
          m_tMax(kRayTMax),
          m_time(0.0f)
    {

    }

    Ray(const Ray& r)
        : m_origin(r.m_origin),
          m_direction(r.m_direction),
          m_tMax(r.m_tMax),
          m_time(r.m_time)
    {

    }

    Ray(const Point& origin, const Vector& direction, float tMax = kRayTMax, float time = 0.0f)
        : m_origin(origin),
          m_direction(direction.normalized()),
          m_tMax(tMax),
          m_time(time)
    {

    }

    Ray& operator =(const Ray& r)
    {
        m_origin = r.m_origin;
        m_direction = r.m_direction;
        m_tMax = r.m_tMax;
        m_time = r.m_time;
        return *this;
    }

    Point calculate(float t) const { return m_origin + t * m_direction; }

    Ray transform(QMatrix4x4 matrix){
        QVector3D orig_point1 = m_origin.toQVector3D();
        Point tmp_orig_point2 = m_origin+m_direction;
        QVector3D orig_point2 = tmp_orig_point2.toQVector3D();
        QVector3D transformed_point1 = matrix * orig_point1;
        QVector3D transformed_point2 = matrix * orig_point2;
        QVector3D transformed_direction = transformed_point2-transformed_point1;
        return Ray(Point(transformed_point1),
                   Vector(transformed_direction.normalized()),
                   m_tMax,
                   m_time);
    }
};

class Model;
class Light;

struct Intersection
{
    Ray m_ray;
    float m_t; // world coordinate
    float m_model_t; // model coordinate
    Model *m_pModel;
    Color m_colorModifier;
    Point m_intersect;
    Point m_model_intersect; // for this model
    Vector m_normal;
    Vector m_model_normal;
    bool intersected;
    bool model_intersected;
    std::vector<Light*> lights; // all light sources

    Intersection()
        : m_ray(),
          m_t(kRayTMax),
          m_model_t(kRayTMax),
          m_pModel(nullptr),
          m_colorModifier(1.0f, 1.0f, 1.0f),
          m_intersect(),
          m_model_intersect(),
          m_normal(),
          m_model_normal(),
          intersected(false),
          model_intersected(false),
          lights()
    {

    }

    Intersection(const Intersection& i)
        : m_ray(i.m_ray),
          m_t(i.m_t),
          m_model_t(i.m_model_t),
          m_pModel(i.m_pModel),
          m_colorModifier(i.m_colorModifier),
          m_intersect(i.m_intersect),
          m_model_intersect(i.m_model_intersect),
          m_normal(i.m_normal),
          m_model_normal(i.m_model_normal),
          intersected(false),
          model_intersected(false),
          lights(i.lights)
    {

    }

    Intersection(const Ray& ray)
         : m_ray(ray),
           m_t(ray.m_tMax),
           m_model_t(ray.m_tMax),
           m_pModel(nullptr),
           m_colorModifier(1.0f, 1.0f, 1.0f),
           m_intersect(),
           m_model_intersect(),
           m_normal(),
           m_model_normal(),
           intersected(false),
           model_intersected(false),
           lights()
    {

    }

    Intersection& operator =(const Intersection& i)
    {
        m_ray = i.m_ray;
        m_t = i.m_t;
        m_model_t = i.m_model_t;
        m_pModel = i.m_pModel;
        m_colorModifier = i.m_colorModifier;
        m_intersect = i.m_intersect;
        m_model_intersect = i.m_model_intersect;
        m_normal = i.m_normal;
        m_model_normal = i.m_model_normal;
        lights = i.lights;
        intersected = i.intersected;
        model_intersected = i.model_intersected;
        return *this;
    }

    Point model_intersect(float t){
        return m_ray.m_origin + t * m_ray.m_direction;
    }
};

#endif // RAYTRACING_H
