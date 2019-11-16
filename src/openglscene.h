#ifndef OpenGLScene_H
#define OpenGLScene_H

#include <vector>

#include <QVector>
#include <QOpenGLWidget>
#include <QOpenGLFunctions_3_3_Core>
#include <QOpenGLShaderProgram>

#include "global.h"
#include "camera.h"
#include "aabb.h"

class Model;
class Scene;
class Light;
class PointLight;

unsigned int loadTexture(const char *path);

class OpenGLScene : public QOpenGLWidget,
        public QOpenGLFunctions_3_3_Core
{
    Q_OBJECT
public:
    enum ProjectionMode {Ortho, Perspective};
    enum ShadowMode {NO_SHADOW, SHADOW_MAPPING, POINT_SHADOW};
    Camera* camera;
    // scene
    Scene *scene;

signals:
    void updateCameraPosition();

public:
    OpenGLScene(QWidget *parent = nullptr);
    ~OpenGLScene() Q_DECL_OVERRIDE;
    void setMode(ProjectionMode mode);
    void setShadowMode(ShadowMode mode);
    void setPhongParams(QVector3D ka, QVector3D ks, double n);
    void setCameraParams(float nearPlane, float farPlane, float speed);
    void resetCameraPose();
    void setLight(Light* light);

protected:
    virtual void initializeGL() Q_DECL_OVERRIDE;
    virtual void resizeGL(int w, int h) Q_DECL_OVERRIDE;
    virtual void paintGL() Q_DECL_OVERRIDE;

    void keyPressEvent(QKeyEvent *event) Q_DECL_OVERRIDE;
    void keyReleaseEvent(QKeyEvent *event) Q_DECL_OVERRIDE;
    void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseReleaseEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseMoveEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void wheelEvent(QWheelEvent *event) Q_DECL_OVERRIDE;

private:
    bool createShader();
    unsigned int loadTexture(char const * path);
    void renderScene(QOpenGLShaderProgram& shader, bool light=true);
    void renderCube();
    void renderQuad();

private:
    // shader
    QOpenGLShaderProgram shadow_mapping_phong_shader;
    QOpenGLShaderProgram shadow_mapping_depth_shader;
    QOpenGLShaderProgram point_shadow_phong_shader;
    QOpenGLShaderProgram point_shadow_depth_shader;

    // objects
    GLuint planeVAO;
    GLuint planeVBO;

    // shadow
    std::vector<GLuint> depthMapFBOv;
    std::vector<GLuint> depthMapv;
    GLuint depthCubeMapFBO;
    GLuint depthCubeMap;

    std::vector<QMatrix4x4> shadowTransforms;

    QTimer* m_pTimer = nullptr;
    int     m_nTimeValue = 0;

    // camera
    bool m_bLeftPressed;
    QPoint m_lastPos;

    // projection
    ProjectionMode mode;
    ShadowMode shadowMode;

    // phong parameters
    QVector3D ka;
    QVector3D ks;
    double n;

    // widget
    int w;
    int h;

    // light
    std::vector<PointLight*> pointLightv; // opengl support only point light

    // models
    std::vector<AABB> objModelAABBs;
};

#endif // OpenGLScene_H
